/*
 *  Copyright (c) 2015 The WebRTC@AnyRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 *
 */

#include "webrtc/modules/video_coding/codecs/h264/h264_decoder_impl.h"

#include <algorithm>
#include <limits>

#ifdef _WIN32 // in win32 we use the vcpkg lib.
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}  // extern "C"
#else

extern "C" {
#include "third_party/ffmpeg/libavcodec/avcodec.h"
#include "third_party/ffmpeg/libavformat/avformat.h"
#include "third_party/ffmpeg/libavutil/imgutils.h"
#include "third_party/ffmpeg/libswscale/swscale.h"
}  // extern "C"
#endif
#include "webrtc/base/checks.h"
#include "webrtc/base/criticalsection.h"
#include "webrtc/base/keep_ref_until_done.h"
#include "webrtc/base/logging.h"
#include "webrtc/system_wrappers/include/metrics.h"

namespace webrtc {

namespace {

const AVPixelFormat kPixelFormat = AV_PIX_FMT_YUV420P;
const size_t kYPlaneIndex = 0;
const size_t kUPlaneIndex = 1;
const size_t kVPlaneIndex = 2;

// Used by histograms. Values of entries should not be changed.
enum H264DecoderImplEvent {
  kH264DecoderEventInit = 0,
  kH264DecoderEventError = 1,
  kH264DecoderEventMax = 16,
};

#if defined(WEBRTC_INITIALIZE_FFMPEG)

rtc::CriticalSection ffmpeg_init_lock;
bool ffmpeg_initialized = false;

// Called by FFmpeg to do mutex operations if initialized using
// |InitializeFFmpeg|.
int LockManagerOperation(void** lock, AVLockOp op)
    EXCLUSIVE_LOCK_FUNCTION() UNLOCK_FUNCTION() {
  switch (op) {
    case AV_LOCK_CREATE:
      *lock = new rtc::CriticalSection();
      return 0;
    case AV_LOCK_OBTAIN:
      static_cast<rtc::CriticalSection*>(*lock)->Enter();
      return 0;
    case AV_LOCK_RELEASE:
      static_cast<rtc::CriticalSection*>(*lock)->Leave();
      return 0;
    case AV_LOCK_DESTROY:
      delete static_cast<rtc::CriticalSection*>(*lock);
      *lock = nullptr;
      return 0;
  }
  RTC_NOTREACHED() << "Unrecognized AVLockOp.";
  return -1;
}

void InitializeFFmpeg() {
  rtc::CritScope cs(&ffmpeg_init_lock);
  if (!ffmpeg_initialized) {
    if (av_lockmgr_register(LockManagerOperation) < 0) {
      RTC_NOTREACHED() << "av_lockmgr_register failed.";
      return;
    }
    av_register_all();
    ffmpeg_initialized = true;
  }
}

#endif  // defined(WEBRTC_INITIALIZE_FFMPEG)

}  // namespace

int H264DecoderImpl::AVGetBuffer2(
    AVCodecContext* context, AVFrame* av_frame, int flags) {
  // Set in |InitDecode|.
  H264DecoderImpl* decoder = static_cast<H264DecoderImpl*>(context->opaque);
  // DCHECK values set in |InitDecode|.
  RTC_DCHECK(decoder);
  //* RTC_DCHECK_EQ(context->pix_fmt, kPixelFormat);	//@Eric - for sometimes AV_PIX_FMT_YUVJ420P
  // Necessary capability to be allowed to provide our own buffers.
  RTC_DCHECK(context->codec->capabilities | AV_CODEC_CAP_DR1);

  // |av_frame->width| and |av_frame->height| are set by FFmpeg. These are the
  // actual image's dimensions and may be different from |context->width| and
  // |context->coded_width| due to reordering.
  int width = av_frame->width;
  int height = av_frame->height;
  // See |lowres|, if used the decoder scales the image by 1/2^(lowres). This
  // has implications on which resolutions are valid, but we don't use it.
  RTC_CHECK_EQ(context->lowres, 0);
  // Adjust the |width| and |height| to values acceptable by the decoder.
  // Without this, FFmpeg may overflow the buffer. If modified, |width| and/or
  // |height| are larger than the actual image and the image has to be cropped
  // (top-left corner) after decoding to avoid visible borders to the right and
  // bottom of the actual image.
  avcodec_align_dimensions(context, &width, &height);

  RTC_CHECK_GE(width, 0);
  RTC_CHECK_GE(height, 0);
  int ret = av_image_check_size(static_cast<unsigned int>(width),
                                static_cast<unsigned int>(height), 0, nullptr);
  if (ret < 0) {
    WCLOG(LS_ERROR) << "Invalid picture size " << width << "x" << height;
    decoder->ReportError();
    return ret;
  }

  // The video frame is stored in |video_frame|. |av_frame| is FFmpeg's version
  // of a video frame and will be set up to reference |video_frame|'s buffers.

  // TODO(nisse): The VideoFrame's timestamp and rotation info is not used.
  // Refactor to do not use a VideoFrame object at all.

  // FFmpeg expects the initial allocation to be zero-initialized according to
  // http://crbug.com/390941. Our pool is set up to zero-initialize new buffers.
  VideoFrame* video_frame = new VideoFrame(
      decoder->pool_.CreateBuffer(width, height),
      0 /* timestamp */, 0 /* render_time_ms */, kVideoRotation_0);

  // DCHECK that we have a continuous buffer as is required.
  RTC_DCHECK_EQ(video_frame->video_frame_buffer()->DataU(),
                video_frame->video_frame_buffer()->DataY() +
                video_frame->allocated_size(kYPlane));
  RTC_DCHECK_EQ(video_frame->video_frame_buffer()->DataV(),
                video_frame->video_frame_buffer()->DataU() +
                video_frame->allocated_size(kUPlane));
  int total_size = video_frame->allocated_size(kYPlane) +
                   video_frame->allocated_size(kUPlane) +
                   video_frame->allocated_size(kVPlane);

  av_frame->format = context->pix_fmt;
  av_frame->reordered_opaque = context->reordered_opaque;

  // Set |av_frame| members as required by FFmpeg.
  av_frame->data[kYPlaneIndex] =
      video_frame->video_frame_buffer()->MutableDataY();
  av_frame->linesize[kYPlaneIndex] =
      video_frame->video_frame_buffer()->StrideY();
  av_frame->data[kUPlaneIndex] =
      video_frame->video_frame_buffer()->MutableDataU();
  av_frame->linesize[kUPlaneIndex] =
      video_frame->video_frame_buffer()->StrideU();
  av_frame->data[kVPlaneIndex] =
      video_frame->video_frame_buffer()->MutableDataV();
  av_frame->linesize[kVPlaneIndex] =
      video_frame->video_frame_buffer()->StrideV();
  RTC_DCHECK_EQ(av_frame->extended_data, av_frame->data);

  av_frame->buf[0] = av_buffer_create(av_frame->data[kYPlaneIndex],
                                      total_size,
                                      AVFreeBuffer2,
                                      static_cast<void*>(video_frame),
                                      0);
  RTC_CHECK(av_frame->buf[0]);
  return 0;
}

void H264DecoderImpl::AVFreeBuffer2(void* opaque, uint8_t* data) {
  // The buffer pool recycles the buffer used by |video_frame| when there are no
  // more references to it. |video_frame| is a thin buffer holder and is not
  // recycled.
  VideoFrame* video_frame = static_cast<VideoFrame*>(opaque);
  delete video_frame;
}

H264DecoderImpl::H264DecoderImpl(const char* p) : 
	pool_(true),
	decoded_image_callback_(nullptr),
	has_reported_init_(false),
	mpath(p),
	has_reported_error_(false) {
}

H264DecoderImpl::~H264DecoderImpl() {
  Release();
}

int32_t H264DecoderImpl::InitDecode(const VideoCodec* codec_settings,
                                    int32_t number_of_cores) {
    i = 0;
  ReportInit();
  if (codec_settings &&
      codec_settings->codecType != kVideoCodecH264) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }

  // FFmpeg must have been initialized (with |av_lockmgr_register| and
  // |av_register_all|) before we proceed. |InitializeFFmpeg| does this, which
  // makes sense for WebRTC standalone. In other cases, such as Chromium, FFmpeg
  // is initialized externally and calling |InitializeFFmpeg| would be
  // thread-unsafe and result in FFmpeg being initialized twice, which could
  // break other FFmpeg usage. See the |rtc_initialize_ffmpeg| flag.
#if defined(WEBRTC_INITIALIZE_FFMPEG)
  // Make sure FFmpeg has been initialized. Subsequent |InitializeFFmpeg| calls
  // do nothing.
  InitializeFFmpeg();
#endif

  // Release necessary in case of re-initializing.
  int32_t ret = Release();
  if (ret != WEBRTC_VIDEO_CODEC_OK) {
    ReportError();
    return ret;
  }
  RTC_DCHECK(!av_context_);

  // Initialize AVCodecContext.
  av_context_.reset(avcodec_alloc_context3(nullptr));

  av_context_->codec_type = AVMEDIA_TYPE_VIDEO;
  av_context_->codec_id = AV_CODEC_ID_H264;
  if (codec_settings) {
    av_context_->coded_width = codec_settings->width;
    av_context_->coded_height = codec_settings->height;
  }
  av_context_->pix_fmt = kPixelFormat;
  av_context_->extradata = nullptr;
  av_context_->extradata_size = 0;
  av_context_->bit_rate = 1920 * 1200 * 3;

  // If this is ever increased, look at |av_context_->thread_safe_callbacks| and
  // make it possible to disable the thread checker in the frame buffer pool.
  av_context_->thread_count = 1;
  av_context_->thread_type = FF_THREAD_SLICE;

  // Function used by FFmpeg to get buffers to store decoded frames in.
  av_context_->get_buffer2 = AVGetBuffer2;
  // |get_buffer2| is called with the context, there |opaque| can be used to get
  // a pointer |this|.
  av_context_->opaque = this;
  // Use ref counted frames (av_frame_unref).
  av_context_->refcounted_frames = 1;  // true

  AVCodec* codec = avcodec_find_decoder(av_context_->codec_id);
  if (!codec) {
    // This is an indication that FFmpeg has not been initialized or it has not
    // been compiled/initialized with the correct set of codecs.
    WCLOG(LS_ERROR) << "FFmpeg H.264 decoder not found.";
    Release();
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  int res = avcodec_open2(av_context_.get(), codec, nullptr);
  if (res < 0) {
    WCLOG(LS_ERROR) << "avcodec_open2 error: " << res;
    Release();
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERROR;
  }

  av_frame_.reset(av_frame_alloc());
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t H264DecoderImpl::Release() {
  av_context_.reset();
  av_frame_.reset();
  
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t H264DecoderImpl::RegisterDecodeCompleteCallback(
    DecodedImageCallback* callback) {
  decoded_image_callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}
#pragma pack(push)
#pragma pack(2)
struct BITMAPFILEHEADER1 {
	unsigned short    bfType;
	unsigned long   bfSize;
	unsigned short    bfReserved1;
	unsigned short    bfReserved2;
	unsigned long   bfOffBits;
};

struct BITMAPINFOHEADER1 {
	unsigned long      biSize;
	long       biWidth;
	long       biHeight;
	unsigned short       biPlanes;
	unsigned short       biBitCount;
	unsigned long      biCompression;
	unsigned long      biSizeImage;
	long       biXPelsPerMeter;
	long       biYPelsPerMeter;
	unsigned long      biClrUsed;
	unsigned long      biClrImportant;
};

#pragma pack(pop)

void H264DecoderImpl::SaveAsBMP(AVFrame *pFrameRGB, int width, int height, int index, int bpp)
{
	char buf[5] = { 0 };
	BITMAPFILEHEADER1 bmpheader;
	BITMAPINFOHEADER1 bmpinfo;
	FILE *fp;

	char *filename = new char[512];

	//文件存放路径，根据自己的修改  
	sprintf(filename, "%s/%d.bmp", mpath.c_str(), index);
	if ((fp = fopen(filename, "wb+")) == NULL) {
		printf("open file failed!\n");
		return;
	}

	bmpheader.bfType = 0x4d42;
	bmpheader.bfReserved1 = 0;
	bmpheader.bfReserved2 = 0;
	bmpheader.bfOffBits = sizeof(bmpheader) + sizeof(bmpinfo);
	bmpheader.bfSize = sizeof(bmpheader);// bmpheader.bfOffBits + width * height*bpp / 8;

	bmpinfo.biSize = sizeof(bmpinfo);
	bmpinfo.biWidth = width;
	bmpinfo.biHeight = height;
	bmpinfo.biPlanes = 1;
	bmpinfo.biBitCount = bpp;
	bmpinfo.biCompression = 0;
	bmpinfo.biSizeImage = (width*bpp + 31) / 32 * 4 * height;
	bmpinfo.biXPelsPerMeter = 100;
	bmpinfo.biYPelsPerMeter = 100;
	bmpinfo.biClrUsed = 0;
	bmpinfo.biClrImportant = 0;

	fwrite(&bmpheader, sizeof(bmpheader), 1, fp);
	fwrite(&bmpinfo, sizeof(bmpinfo), 1, fp);
	fwrite(pFrameRGB->data[0], width*height*bpp / 8, 1, fp);

	fclose(fp);
}

int32_t H264DecoderImpl::Decode(const EncodedImage& input_image,
                                bool /*missing_frames*/,
                                const RTPFragmentationHeader* /*fragmentation*/,
                                const CodecSpecificInfo* codec_specific_info,
                                int64_t /*render_time_ms*/) {

  if (!IsInitialized()) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
  if (!decoded_image_callback_) {
    WCLOG(LS_WARNING) << "InitDecode() has been called, but a callback function "
        "has not been set with RegisterDecodeCompleteCallback()";
    ReportError();
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
  if (!input_image._buffer || !input_image._length) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }
  if (codec_specific_info &&
      codec_specific_info->codecType != kVideoCodecH264) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }

  // FFmpeg requires padding due to some optimized bitstream readers reading 32
  // or 64 bits at once and could read over the end. See avcodec_decode_video2.
  RTC_CHECK_GE(input_image._size, input_image._length +
                   EncodedImage::GetBufferPaddingBytes(kVideoCodecH264));
  // "If the first 23 bits of the additional bytes are not 0, then damaged MPEG
  // bitstreams could cause overread and segfault." See
  // AV_INPUT_BUFFER_PADDING_SIZE. We'll zero the entire padding just in case.
  memset(input_image._buffer + input_image._length,
         0,
         EncodedImage::GetBufferPaddingBytes(kVideoCodecH264));

  AVPacket packet;
  av_init_packet(&packet);
  packet.data = input_image._buffer;
  if (input_image._length > static_cast<size_t>(std::numeric_limits<int>::max())) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  packet.size = static_cast<int>(input_image._length);
  av_context_->reordered_opaque = input_image.ntp_time_ms_ * 1000;  // ms -> μs

  //int frame_decoded = 0;
  int result = -1;
  if (avcodec_send_packet(av_context_.get(), &packet) == 0) {
	  while (true) {
		  result = avcodec_receive_frame(av_context_.get(), av_frame_.get());
		  break;
	  }
  }
  //int result = avcodec_decode_video2(av_context_.get(), av_frame_.get(), &frame_decoded, &packet);
  if (result != 0) {
    WCLOG(LS_ERROR) << "avcodec_decode_video2 error: " << result;
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
  
  // |result| is number of bytes used, which should be all of them.
  //if (result != packet.size) {
  //  WCLOG(LS_ERROR) << "avcodec_decode_video2 consumed " << result << " bytes "
  //      "when " << packet.size << " bytes were expected.";
  //  ReportError();
  //  return WEBRTC_VIDEO_CODEC_ERROR;
  //}

 // if (!frame_decoded) {
  //  WCLOG(LS_ERROR) << "avcodec_decode_video2 successful but no frame was "
//        "decoded.";
 //   return WEBRTC_VIDEO_CODEC_OK;
//  }

/*
  
  AVFrame* pFrameRGB = av_frame_alloc();

  struct SwsContext *pSwsCtx;
  //AVPacket packet;
  int frameFinished;
  int PictureSize;
  uint8_t *outBuff;

  PictureSize = avpicture_get_size(AV_PIX_FMT_BGR24, av_context_.get()->width, av_context_.get()->height);
  outBuff = (uint8_t*)av_malloc(PictureSize);
  if (outBuff == NULL) {
	  printf("av malloc failed!\n");
	  exit(1);
  }
  avpicture_fill((AVPicture *)pFrameRGB, outBuff, AV_PIX_FMT_BGR24, av_context_.get()->width, av_context_.get()->height);

  //设置图像转换上下文  
  pSwsCtx = sws_getContext(av_context_.get()->width, av_context_.get()->height, av_context_.get()->pix_fmt,
	  av_context_.get()->width, av_context_.get()->height, AV_PIX_FMT_BGR24,
	  SWS_BICUBIC, NULL, NULL, NULL);


  sws_scale(pSwsCtx, av_frame_.get()->data,
	  av_frame_.get()->linesize, 0, av_context_.get()->height,
	  pFrameRGB->data, pFrameRGB->linesize);

  SaveAsBMP(pFrameRGB, av_context_.get()->width, av_context_.get()->height, i++, 24);
*/

  // Obtain the |video_frame| containing the decoded image.
  VideoFrame* video_frame = static_cast<VideoFrame*>(
      av_buffer_get_opaque(av_frame_->buf[0]));
  RTC_DCHECK(video_frame);
  RTC_CHECK_EQ(av_frame_->data[kYPlane],
               video_frame->video_frame_buffer()->DataY());
  RTC_CHECK_EQ(av_frame_->data[kUPlane],
               video_frame->video_frame_buffer()->DataU());
  RTC_CHECK_EQ(av_frame_->data[kVPlane],
               video_frame->video_frame_buffer()->DataV());
  video_frame->set_timestamp(input_image._timeStamp);

  int32_t ret;

  // The decoded image may be larger than what is supposed to be visible, see
  // |AVGetBuffer2|'s use of |avcodec_align_dimensions|. This crops the image
  // without copying the underlying buffer.
  rtc::scoped_refptr<VideoFrameBuffer> buf = video_frame->video_frame_buffer();
	//WCLOG(LS_WARNING) << "DecodedImageCallback::Decoded width:" << buf->width() << "height:" << buf->height();
  if (av_frame_->width != buf->width() || av_frame_->height != buf->height()) {
     // WCLOG(LS_ERROR) << "Changesize (" << av_frame_->width << "," << av_frame_->height << ") (" << buf->width() << "," << buf->height() << ")";
    rtc::scoped_refptr<VideoFrameBuffer> cropped_buf(
        new rtc::RefCountedObject<WrappedI420Buffer>(
            av_frame_->width, av_frame_->height,
            buf->DataY(), buf->StrideY(),
            buf->DataU(), buf->StrideU(),
            buf->DataV(), buf->StrideV(),
            rtc::KeepRefUntilDone(buf)));
    VideoFrame cropped_frame(
        cropped_buf, video_frame->timestamp(), video_frame->render_time_ms(),
        video_frame->rotation());
    // TODO(nisse): Timestamp and rotation are all zero here. Change decoder
    // interface to pass a VideoFrameBuffer instead of a VideoFrame?
    ret = decoded_image_callback_->Decoded(cropped_frame);
  } else {
     // WCLOG(LS_ERROR) << "Changesize ok";
    // Return decoded frame.
    ret = decoded_image_callback_->Decoded(*video_frame);
  }
  // Stop referencing it, possibly freeing |video_frame|.
  av_frame_unref(av_frame_.get());
  video_frame = nullptr;

  if (ret) {
    WCLOG(LS_ERROR) << "DecodedImageCallback::Decoded returned " << ret;
    return ret;
  }
  return WEBRTC_VIDEO_CODEC_OK;
}

const char* H264DecoderImpl::ImplementationName() const {
  return "FFmpeg";
}

bool H264DecoderImpl::IsInitialized() const {
  return av_context_ != nullptr;
}

void H264DecoderImpl::ReportInit() {
  if (has_reported_init_)
    return;
  RTC_HISTOGRAM_ENUMERATION("WebRTC.Video.H264DecoderImpl.Event",
                            kH264DecoderEventInit,
                            kH264DecoderEventMax);
  has_reported_init_ = true;
}

void H264DecoderImpl::ReportError() {
  if (has_reported_error_)
    return;
  RTC_HISTOGRAM_ENUMERATION("WebRTC.Video.H264DecoderImpl.Event",
                            kH264DecoderEventError,
                            kH264DecoderEventMax);
  has_reported_error_ = true;
}

}  // namespace webrtc
