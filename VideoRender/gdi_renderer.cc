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
#include "../Prj-Win/xdefines.h"

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
		hwnd_(NULL){
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
		PostMessage(GetParent(hwnd_), WM_PULLDLG_RESIZE, width+40, height+20);
	}
	int bmp_write(unsigned char *image, int imageWidth, int imageHeight, char *filename){
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

	void DrawBitmap(HWND hwnd, int w0, int h0, const unsigned char *pBmpData){
		RECT rc;
		double x, y, h3, w3;
		GetClientRect(hwnd, &rc);
		int totalh = rc.bottom - rc.top;
		int totalw = rc.right - rc.left;

		rc.bottom -= 10;
		rc.right -= 20;
		
		int h1, w1;
		h1 = rc.bottom - rc.top;
		w1 = rc.right - rc.left;
		h1 = h1 < h0 ? h0 : h1;
		w1 = w1 < w0 ? w0 : w1;
		if ((h1 > (rc.bottom - rc.top)) || (w1 > (rc.right - rc.left))) {
			PostMessage(GetParent(hwnd), WM_PULLDLG_RESIZE, w0 + 20, h0 + 40);
			return;
		}

		int h2 = h0 * w1 / w0;
		int w2 = w0 * h1 / h0;
		h3 = h2;
		w3 = h2 * w0 / h0;
		if (h2 > (rc.bottom - rc.top)) {
			w3 = w2;
			h3 = w2 * h0 / w0;
		}

		//if (IsZoomed(hwnd) == 0) {
		//	w3 = w0;
		//	h3 = h0;
		//}
		

		//w3 = (double)w0 * 1.25f;
		//h3 = (double)h0 * 1.25f;

		if (w3 > (rc.right - rc.left)) {
			w3 = w0;
			h3 = h0;
		}

		if (w3 < w0 + 50) {
			w3 = w0;
			h3 = h0;
		}

		x = (totalw - w3) / 2;
		y = (totalh - h3) / 2;
		x = x < 0 ? 0 : x;
		y = y < 0 ? 0 : y;


		HBITMAP hBitmap = ::CreateBitmap(w0, h0, 1, 32, pBmpData);
		HDC hWndDc = ::GetDC(hwnd);
		HDC hMemDc = ::CreateCompatibleDC(hWndDc);
		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDc, hBitmap);
		//::BitBlt(hWndDc, x, y, w3, h3, hMemDc, 0, 0, SRCCOPY);
		int mode = ::SetStretchBltMode(hWndDc,HALFTONE);
		::SetBrushOrgEx(hWndDc, 0, 0, NULL);
		//SetStretchBltMode(COLORONCOLOR);
		::StretchBlt(hWndDc,x, y, w3, h3, hMemDc, 0, 0, w0, h0, SRCCOPY);
		::SetStretchBltMode(hWndDc, mode);
		::SetBrushOrgEx(hWndDc, 0, 0, NULL);
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
		DrawBitmap(hwnd_,width_, height_, pBits);
		//char filename[1024] = { 0 };
		//sprintf(filename, "test%08d", fps_);
		//bmp_write((unsigned char*)pBits, 512, 512, filename);
		delete pBits;

	}
}  // namespace webrtc
