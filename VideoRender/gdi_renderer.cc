/*
 *  Copyright (c) 2013 The WebRTC@AnyRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "gdi_renderer.h"

#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"

namespace webrtc {

	const char kGdiClassName[] = "gdi_renderer";

#if 1
	VideoRenderer* VideoRenderer::CreatePlatformRenderer(const void* hwnd,
		size_t width,
		size_t height) {
		return GdiRenderer::Create(hwnd, width, height);
	}
#endif

	GdiRenderer::GdiRenderer(size_t width, size_t height)
		: width_(width),
		height_(height),
		fps_(0),
		hwnd_(NULL)
	{
		assert(width > 0);
		assert(height > 0);
	}

	GdiRenderer::~GdiRenderer() { Destroy(); }

	void GdiRenderer::Destroy() {

		if (hwnd_ != NULL) {
			hwnd_ = NULL;
		}
	}

	bool GdiRenderer::Init(const void* hwnd) {
		hwnd_ = (HWND)hwnd;

		if (hwnd_ == NULL) {
			return false;
		}

		Resize(width_, height_);
		return true;
	}

	GdiRenderer* GdiRenderer::Create(const void*hwnd,
		size_t width,
		size_t height) {
		GdiRenderer* gdi_renderer = new GdiRenderer(width, height);
		if (!gdi_renderer->Init(hwnd)) {
			delete gdi_renderer;
			return NULL;
		}

		return gdi_renderer;
	}

	void GdiRenderer::Resize(size_t width, size_t height) {
		width_ = width;
		height_ = height;

		::SetWindowPos(hwnd_, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
		RECT rc;
		::GetClientRect(hwnd_, &rc);

	}


	//test function for save local bmp
	int bmp_write(unsigned char *image, int imageWidth, int imageHeight, char *filename)
	{
		unsigned char header[54] = {
		  0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0,
			54, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 32, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0
		};

		long file_size = (long)imageWidth * (long)imageHeight * 4 + 54;
		header[2] = (unsigned char)(file_size & 0x000000ff);
		header[3] = (file_size >> 8) & 0x000000ff;
		header[4] = (file_size >> 16) & 0x000000ff;
		header[5] = (file_size >> 24) & 0x000000ff;

		long width = imageWidth;
		header[18] = width & 0x000000ff;
		header[19] = (width >> 8) & 0x000000ff;
		header[20] = (width >> 16) & 0x000000ff;
		header[21] = (width >> 24) & 0x000000ff;

		long height = 0-imageHeight;
		header[22] = height & 0x000000ff;
		header[23] = (height >> 8) & 0x000000ff;
		header[24] = (height >> 16) & 0x000000ff;
		header[25] = (height >> 24) & 0x000000ff;

		char fname_bmp[128];
		sprintf(fname_bmp, "c:/temp/%s.bmp", filename);

		FILE *fp;
		if (!(fp = fopen(fname_bmp, "wb")))
			return -1;

		fwrite(header, sizeof(unsigned char), 54, fp);
		fwrite(image, sizeof(unsigned char), (size_t)(long)imageWidth * imageHeight * 4, fp);

		fclose(fp);
		return 0;
	}

	void DrawBitmap(HWND hwnd, int x, int y, int nBmpWidth, int nBmpHeight, const unsigned char *pBmpData)
	{
		HBITMAP hBitmap = ::CreateBitmap(nBmpWidth, nBmpHeight, 1, 32, pBmpData);
		HDC hWndDc = ::GetDC(hwnd);
		HDC hMemDc = ::CreateCompatibleDC(hWndDc);
		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDc, hBitmap);
		::BitBlt(hWndDc, x, y, nBmpWidth, nBmpHeight, hMemDc, 0, 0, SRCCOPY);

		::SelectObject(hMemDc, hOldBitmap);
		::DeleteObject(hBitmap);
		::DeleteDC(hMemDc);
		::ReleaseDC(hwnd, hWndDc);
	}

	void GdiRenderer::OnFrame(const cricket::VideoFrame& frame) {

		fps_++;
		const cricket::VideoFrame*videoFrame = &frame;
		if (static_cast<size_t>(videoFrame->width()) != width_ ||
			static_cast<size_t>(videoFrame->height()) != height_) {
			Resize(static_cast<size_t>(videoFrame->width()),
				static_cast<size_t>(videoFrame->height()));
		}

		unsigned char * pBits = new unsigned char[width_ * height_ * 4];

		webrtc::VideoFrame video_frame(videoFrame->video_frame_buffer(), 0, 0, videoFrame->rotation());
		ConvertFromI420(video_frame, kARGB, 0, static_cast<uint8_t*>(pBits));
		DrawBitmap(hwnd_, 0, 0, width_, height_, pBits);
		//char filename[1024] = { 0 };
		//sprintf(filename, "test%08d", fps_);
		//bmp_write((unsigned char*)pBits, 512, 512, filename);
		delete pBits;

	}
}  // namespace webrtc
