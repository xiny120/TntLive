/*
*  Copyright (c) 2016 The AnyRTC project authors. All Rights Reserved.
*
*  Please visit https://www.anyrtc.io for detail.
*
* The GNU General Public License is a free, copyleft license for
* software and other kinds of works.
*
* The licenses for most software and other practical works are designed
* to take away your freedom to share and change the works.  By contrast,
* the GNU General Public License is intended to guarantee your freedom to
* share and change all versions of a program--to make sure it remains free
* software for all its users.  We, the Free Software Foundation, use the
* GNU General Public License for most of our software; it applies also to
* any other work released this way by its authors.  You can apply it to
* your programs, too.
* See the GNU LICENSE file for more info.
*/
#include "plybuffer.h"
#include "webrtc/base/logging.h"
#include "plydecoder.h"


#define PLY_MIN_TIME	500		// 0.5s
#define PLY_MAX_TIME	600000		// 10minute
#define PLY_RED_TIME	200		// redundancy time org 250
#define PLY_2LOW_TIME 100 * 15	// 播放延迟太长了。缓冲了3秒数据了。滞后3秒。
#define PLY_MAX_DELAY	1000		// 1 second
#define PLY_MAX_CACHE   30      	// 16s

#define PB_TICK	1011


PlyBuffer::PlyBuffer(PlyBufferCallback&callback, rtc::Thread*worker)
	: callback_(callback)
	, worker_thread_(NULL)
	, got_audio_(false)
	, cache_time_(500)	// default 1000ms(1s)
	, cache_delta_(1)
	, buf_cache_time_(100)
	, ply_status_(PS_Fast)
	, sys_fast_video_time_(0)
	, rtmp_fast_video_time_(0)
	, rtmp_cache_time_(0)
	, play_cur_time_(0)
	, gofast(false)
	, gofast0(true)
	, fastbuf(0)
	, fastbuflen(0)
	, fastbufleft(0)
	, fastbufi(0)
	, state(0)
	, samp_s(280)
	, samp_d(240)
{
	ASSERT(worker != NULL);
	PlyDecoder* pd = (PlyDecoder*)worker;

	worker_thread_ = worker;
	worker_thread_->PostDelayed(RTC_FROM_HERE, 1, this, PB_TICK);
}

void PlyBuffer::InitAudio(int samples, int chan_) {
	hz = samples;
	chan = chan_;

	int err = -1;
	state = speex_resampler_init(2, //spx_uint32_t nb_channels,
		hz,//samp_s,//hz, //spx_uint32_t in_rate,
		hz*samp_d/samp_s,// hz * 4 / 5, //spx_uint32_t out_rate,
		10,     //int quality,
		&err    //int *err
	);
}


PlyBuffer::~PlyBuffer()
{
	std::list<PlyPacket*>::iterator iter = lst_audio_buffer_.begin();
	while (iter != lst_audio_buffer_.end()) {
		PlyPacket* pkt = *iter;
		lst_audio_buffer_.erase(iter++);
		delete pkt;
	}
	iter = lst_video_buffer_.begin();
	while (iter != lst_video_buffer_.end()) {
		PlyPacket* pkt = *iter;
		lst_video_buffer_.erase(iter++);
		delete pkt;
	}
	speex_resampler_destroy(state);
}

void PlyBuffer::SetCacheSize(int miliseconds/*ms*/)
{
	if (miliseconds > PLY_MIN_TIME && miliseconds <= PLY_MAX_TIME) {	//* 0.5s ~ 10 minute
		cache_time_ = miliseconds;
	}
}

int gcd(int x, int y)

{

	while (x != y)

	{

		if (x > y) x = x - y;

		else

			y = y - x;

	}

	return x;

}
int PlyBuffer::GetPlayAudio(void* audioSamples,const int samples,const int chan)
{
	int ret = 0;
	rtc::CritScope cs(&cs_list_audio_);
	if (lst_audio_buffer_.size() > 0) {
		//if (lst_audio_buffer_.size() < 50) {
	//		gofast = false;
	//		gofast0 = true;
	//	}
		if (gofast) {
			// 2秒声音用1秒播放，说话速度快一倍。3秒声音用2秒播放，说话速度快0.5倍？
			// 4秒声音用3秒播放，速度快0.25倍？5变4
			// 480 500 12000, 480 510 8160, 480 520 6240,  480 540 4320, 480 560 3360
			std::list<PlyPacket*>::iterator iter = lst_audio_buffer_.begin();
			unsigned int len = 0;
			int i = 0;
			int total = samp_d /gcd(samp_d, samp_s) * samp_s / samp_d;
			for (i = 0; i < total; i++) {
				len += (*iter)->_data_len;
				iter++;
			}
			if (fastbuflen < len) {
				fastbuflen = len;
				delete[] fastbuf;
				fastbufcur = fastbuf = new char[fastbuflen];
			}
			//char* pcur = (char*)fastbuf;
			len = fastbufcur - fastbuf ;
			for (i = 0; i < total; i++) {
				PlyPacket* pkt_front = lst_audio_buffer_.front();
				ret = pkt_front->_data_len;
				play_cur_time_ = pkt_front->_dts;
				memcpy(fastbufcur, pkt_front->_data, pkt_front->_data_len);
				len += pkt_front->_data_len;
				fastbufcur += pkt_front->_data_len;
				lst_audio_buffer_.pop_front();
				delete pkt_front;
				if (len >= samp_s * 4)
					break;
			}

			//len = len / 2;
			unsigned int pLen = samp_s,inlen = samp_s ;
			ret = speex_resampler_process_interleaved_int(state, (short*)fastbuf, &inlen, (short*)audioSamples, &pLen);
			//LOG(LS_WARNING) << "speex_resampler ret:" << ret << " len:" << inlen << " - outlen:" << pLen;
			ret = pLen*4;
			memmove(fastbuf, &fastbuf[samp_s * 4], len - samp_s * 4);
			fastbufcur = &fastbuf[(len - samp_s * 4)];
			//fwrite(outBuffer, 4, pLen, out);
			
		}
		else {
			PlyPacket* pkt_front = lst_audio_buffer_.front();
			ret = pkt_front->_data_len;
			play_cur_time_ = pkt_front->_dts;
			memcpy(audioSamples, pkt_front->_data, pkt_front->_data_len);
			lst_audio_buffer_.pop_front();
			delete pkt_front;
		}
	}

	return ret;
}
void PlyBuffer::CacheH264Data(const uint8_t*pdata, int len, uint32_t ts)
{
	PlyPacket* pkt = new PlyPacket(true);
	pkt->SetData(pdata, len, ts);
	if (sys_fast_video_time_ == 0)
	{
		sys_fast_video_time_ = rtc::Time();
		rtmp_fast_video_time_ = ts;
	}
	rtc::CritScope cs(&cs_list_video_);
	lst_video_buffer_.push_back(pkt);
	//LOG(LS_WARNING) << " CacheH264Data" << len << " - " << ts;
}

void PlyBuffer::CachePcmData(const uint8_t*pdata, int len, uint32_t ts)
{
	//LOG(LS_WARNING) << " CachePcmData " << len << " - " << ts;
	PlyPacket* pkt = new PlyPacket(false);
	pkt->SetData(pdata, len, ts);
	rtc::CritScope cs(&cs_list_audio_);
	got_audio_ = true;
	lst_audio_buffer_.push_back(pkt);
	if (sys_fast_video_time_ == 0) {
		PlyPacket* pkt_front = lst_audio_buffer_.front();
		PlyPacket* pkt_back = lst_audio_buffer_.back();
		if ((pkt_back->_dts - pkt_front->_dts) >= PLY_MAX_DELAY) {
			sys_fast_video_time_ = rtc::Time();
			rtmp_fast_video_time_ = ts;
		}
	}
}

void PlyBuffer::OnMessage(rtc::Message* msg)
{
	if (msg->message_id == PB_TICK) {
		DoDecode();
		worker_thread_->PostDelayed(RTC_FROM_HERE, 5, this, PB_TICK);
	}
}

int	PlyBuffer::GetCacheTime()
{
	return cache_time_;
}

void PlyBuffer::DoDecode()
{
	uint32_t curTime = rtc::Time();
	if (sys_fast_video_time_ == 0)
		return;
	if (ply_status_ == PS_Fast) {
		PlyPacket* pkt = NULL;
		uint32_t videoSysGap = curTime - sys_fast_video_time_;
		uint32_t videoPlyTime = rtmp_fast_video_time_ + videoSysGap;
		if (videoSysGap >= PLY_RED_TIME) {
			//* Start play a/v
			rtc::CritScope cs(&cs_list_audio_);
			if (lst_audio_buffer_.size() > 0) {
				PlyPacket* pkt_front = lst_audio_buffer_.front();
				PlyPacket* pkt_back = lst_audio_buffer_.back();
				if ((pkt_back->_dts - pkt_front->_dts) > PLY_RED_TIME) {
					ply_status_ = PS_Normal;
					play_cur_time_ = pkt_front->_dts;
					callback_.OnPlay();
				}
			}
			else {
				if (videoSysGap >= PLY_RED_TIME * 4)
				{
					rtc::CritScope cs(&cs_list_video_);
					if (lst_video_buffer_.size() > 0) {
						PlyPacket* pkt_front = lst_video_buffer_.front();
						ply_status_ = PS_Normal;
						play_cur_time_ = pkt_front->_dts;
						callback_.OnPlay();
					}
				}
			}
		}
	}
	else if (ply_status_ == PS_Normal) {
		PlyPacket* pkt_video = NULL;
		uint32_t media_buf_time = 0;
		uint32_t play_video_time = play_cur_time_;
		{//* Get audio 
			rtc::CritScope cs(&cs_list_audio_);
			if (lst_audio_buffer_.size() > 0) {
				media_buf_time = lst_audio_buffer_.back()->_dts - lst_audio_buffer_.front()->_dts;
			}
		}
		if (media_buf_time == 0 && !got_audio_) {
			rtc::CritScope cs(&cs_list_video_);
			if (lst_video_buffer_.size() > 0) {
				media_buf_time = lst_video_buffer_.back()->_dts - lst_video_buffer_.front()->_dts;
				uint32_t videoSysGap = curTime - sys_fast_video_time_;
				play_video_time = rtmp_fast_video_time_ + videoSysGap;
			}
		}

		{//* Get video 
			rtc::CritScope cs(&cs_list_video_);
			if (lst_video_buffer_.size() > 0) {
				pkt_video = lst_video_buffer_.front();
				if (pkt_video->_dts <= play_video_time) {
					lst_video_buffer_.pop_front();
				}
				else {
					pkt_video = NULL;
				}
			}
		}

		if (pkt_video) {
			if (!callback_.OnNeedDecodeData(pkt_video)) {
				delete pkt_video;
			}
		}

		LOG(LS_WARNING) << "lst_audio_buffer_.size " << lst_audio_buffer_.size()
			<< " lst_video_buffer_.size " << lst_video_buffer_.size()
			<< " media_buf_time " << media_buf_time;

		if (media_buf_time <= PLY_RED_TIME) {
			LOG(LS_WARNING) << "media_buf_time <= PLY_RED_TIME";
			// Play buffer is so small, then we need buffer it?
			callback_.OnPause();
			ply_status_ = PS_Cache;
			cache_time_ = cache_delta_ * 100;
			if (cache_delta_ < PLY_MAX_CACHE)
				cache_delta_ *= 2;
			if (cache_delta_ >= PLY_MAX_CACHE)
				cache_delta_ = PLY_MAX_CACHE;
			rtmp_cache_time_ = rtc::Time() + cache_time_;
		}
		else if(media_buf_time >= PLY_2LOW_TIME) {
			{
				//rtc::CritScope cs(&cs_list_audio_);
				if (gofast0) {
					gofast = true;
					gofast0 = false;
				}
			}

		}
		else if (media_buf_time <= PLY_RED_TIME * 2) {
			gofast0 = true;
			gofast = false;
		}
		buf_cache_time_ = media_buf_time;
	}
	else if (ply_status_ == PS_Cache) {
		if (rtmp_cache_time_ <= rtc::Time()) {
			uint32_t media_buf_time = 0;
			{
				rtc::CritScope cs(&cs_list_audio_);
				if (lst_audio_buffer_.size() > 0) {
					media_buf_time = lst_audio_buffer_.back()->_dts - lst_audio_buffer_.front()->_dts;
					//LOG(LS_WARNING) << "media_buf_time " << media_buf_time;
				}
			}
			if (media_buf_time == 0 && !got_audio_) {
				rtc::CritScope cs(&cs_list_video_);
				if (lst_video_buffer_.size() > 0) {
					media_buf_time = lst_video_buffer_.back()->_dts - lst_video_buffer_.front()->_dts;
				}
			}

			int cache_time_ts = cache_time_ - PLY_RED_TIME;
			if (cache_time_ts < 0) cache_time_ts = 0;

			if (media_buf_time >= cache_time_ts) {
				ply_status_ = PS_Normal;
				if (cache_delta_ >= PLY_MAX_CACHE)
					cache_delta_ /= 2;
				callback_.OnPlay();
			}
			else {
				rtmp_cache_time_ = rtc::Time() + cache_time_;
			}
			if (!got_audio_) {
				sys_fast_video_time_ += cache_time_;
			}
			buf_cache_time_ = media_buf_time;
		}
	}
}
