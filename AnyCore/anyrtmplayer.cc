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
#include "anyrtmplayer.h"
#include "anyrtmpcore.h"
#include "srs_librtmp.h"
#include "webrtc/base/logging.h"
#include "webrtc/media/base/videoframe.h"
#include "webrtc/modules/audio_device/audio_device_impl.h"
#include "webrtc/voice_engine/voice_engine_defines.h"
#include "httpclient.h"

#define PLY_START	1001
#define PLY_STOP	1002
#define PLY_TICK    1003
#define PLY_SEEK	1004

AnyRtmplayer* AnyRtmplayer::Create(AnyRtmplayerEvent&callback){
	return new webrtc::AnyRtmplayerImpl(callback);
}
namespace webrtc {

AnyRtmplayerImpl::AnyRtmplayerImpl(AnyRtmplayerEvent&callback)	: AnyRtmplayer(callback)
	,m1stAudio(false)
	,m1stVideo(false)
	, rtmp_pull_(NULL)
	, ply_decoder_(NULL)
    , cur_bitrate_(0)
	, mseekpos(0)
	, video_renderer_(NULL){
	rtc::Thread::Start();
	AnyRtmpCore::Inst();
}

AnyRtmplayerImpl::~AnyRtmplayerImpl(void){
	rtc::Thread::Stop();
	if (rtmp_pull_) {
		delete rtmp_pull_;
		rtmp_pull_ = NULL;
	}

	if (ply_decoder_) {
		delete ply_decoder_;
		ply_decoder_ = NULL;
	}
}

uint32_t AnyRtmplayerImpl::SeekTo(uint32_t pos,double totaltime) {
	mseekpos = pos;
	mtotaltime = totaltime;
	rtc::Thread::Clear(this, PLY_TICK);
	rtc::Thread::PostDelayed(RTC_FROM_HERE,1000, this, PLY_SEEK);
	return 0;
}

void AnyRtmplayerImpl::StartPlay(const char* url,const char* type,const char* dir,int32_t mencryption){
	m1stAudio = true;
	m1stVideo = true;
	str_url_ = url;
	mtype = type;
	mdir = dir;
	mmencryption = mencryption;
	rtc::Thread::Post(RTC_FROM_HERE, this, PLY_START);
    rtc::Thread::PostDelayed(RTC_FROM_HERE, 1000, this, PLY_TICK);
}

void AnyRtmplayerImpl::SetVideoRender(void* handle){
	video_renderer_ = (rtc::VideoSinkInterface < cricket::VideoFrame >	*) handle;
}

void AnyRtmplayerImpl::StopPlay(){
    rtc::Thread::Clear(this, PLY_TICK);
	rtc::Thread::Post(RTC_FROM_HERE, this, PLY_STOP);
    callback_.OnRtmplayerClose(0);
}

void AnyRtmplayerImpl::OnMessage(rtc::Message* msg){
	switch (msg->message_id) {
	case PLY_SEEK: {
		if (rtmp_pull_ != NULL) {
			rtmp_pull_->SeekTo(mseekpos,mtotaltime);
			rtc::Thread::PostDelayed(RTC_FROM_HERE, 100, this, PLY_TICK);
			//OnRtmpullResetTime(mseekpos*100000);
		}

	}break;
	case PLY_START: {
		m1stAudio = true;
		m1stVideo = true;
		if (ply_decoder_ == NULL) {
			bool needgofast = true;
			if (strcmp(mtype.c_str(), "rtmp") == 0) {
			}
			else if (strcmp(mtype.c_str(), "flv") == 0) {
				needgofast = false;
			}
			else {
				assert(0);
			}
			ply_decoder_ = new PlyDecoder(needgofast);
			if (video_renderer_)
				ply_decoder_->SetVideoRender(video_renderer_);
		}
		if (rtmp_pull_ == NULL) {
			rtmp_pull_ = new AnyRtmpPull(*this, str_url_.c_str(), mtype.c_str(),mdir.c_str(),mmencryption);
		}
		callback_.OnRtmplayerPlayStart();
	}
		break;
	case PLY_STOP: {
		callback_.OnRtmplayerPlayStop();
		if (rtmp_pull_) {
			delete rtmp_pull_;
			rtmp_pull_ = NULL;
		}
		if (ply_decoder_) {
			delete ply_decoder_;
			ply_decoder_ = NULL;
		}
	}
		break;
    case PLY_TICK: {
        if (ply_decoder_ && rtmp_pull_) {
            if (ply_decoder_->IsPlaying()) {
                callback_.OnRtmplayerStatus(ply_decoder_->CacheTime(), cur_bitrate_, ply_decoder_->CurTime(),rtmp_pull_->GetTotalTime());
                cur_bitrate_ = 0;
            } else {
                callback_.OnRtmplayerCache(ply_decoder_->CacheTime());
            }
        }
        rtc::Thread::PostDelayed(RTC_FROM_HERE, 1000, this, PLY_TICK);
    }
        break;
	}
}

void AnyRtmplayerImpl::OnRtmpullConnected(){
	m1stVideo = true;
	m1stAudio = true;
	callback_.OnRtmplayerOK();
}

void AnyRtmplayerImpl::OnRtmpullFailed(){
	StopPlay();
	callback_.OnRtmplayerClose(-1);
}

void AnyRtmplayerImpl::OnRtmpullDisconnect(){
	StopPlay();
	callback_.OnRtmplayerClose(-2);
}
void AnyRtmplayerImpl::OnRtmpullResetTime(uint32_t t) {
	if (ply_decoder_) {
		ply_decoder_->ResetCurTime(t);
	}
}

void AnyRtmplayerImpl::OnRtmpullH264Data(const uint8_t*pdata, int len, uint32_t ts){
	if (ply_decoder_) {
		ply_decoder_->AddH264Data(pdata, len, ts);
		if (m1stVideo) {
			m1stVideo = false;
			callback_.OnRtmplayer1stVideo();
		}
	}
    cur_bitrate_ += len;
}

void AnyRtmplayerImpl::OnRtmpullAACData(const uint8_t*pdata, int len, uint32_t ts){
	if (ply_decoder_) {
		ply_decoder_->AddAACData(pdata, len, ts);
		if (m1stAudio) {
			m1stAudio = false;
			callback_.OnRtmplayer1stAudio();
		}
	}
    cur_bitrate_ += len;
}

int AnyRtmplayerImpl::GetNeedPlayAudio(void* audioSamples, uint32_t& samplesPerSec, size_t& nChannels){
	if (ply_decoder_) {
		return ply_decoder_->GetPcmData(audioSamples, samplesPerSec, nChannels);
	}
	return 0;
}

bool AnyRtmplayerImpl::OnRtmpullSlowdown() {
	if (ply_decoder_) {
		return ply_decoder_->Slowdown();
	}
	return false;
}

void AnyRtmplayerImpl::OnRtmpullConnectionFailed(int sta) {
	callback_.OnRtmplayerConnectionFailed(sta);
}

}	// namespace webrtc