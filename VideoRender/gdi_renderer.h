/*
 *  Copyright (c) 2013 The WebRTC@AnyRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef WEBRTC_TEST_WIN_GDI_RENDERER_H_
#define WEBRTC_TEST_WIN_GDI_RENDERER_H_

#include <WinSock2.h>


#include "video_renderer.h"
#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/typedefs.h"

namespace webrtc {

	class GdiRenderer : public VideoRenderer {
	public:
		static GdiRenderer* Create(const void* hwnd, size_t width,
			size_t height);
		virtual ~GdiRenderer();

		void OnFrame(const cricket::VideoFrame& frame) override;

	private:
		GdiRenderer(size_t width, size_t height);

		bool Init(const void* hwnd);
		void Resize(size_t width, size_t height);
		void Destroy();

		size_t width_, height_;
		int fps_;

		HWND hwnd_;

	};
}  // namespace webrtc

#endif  // WEBRTC_TEST_WIN_D3D_RENDERER_H_
