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
#include "RtmpGuesterImpl.h"
#include "httpclient.h"
#include "AnyFlvSource.h"

RTMPGuester* RTMPGuester::Create(RTMPGuesterEvent&callback){
	return new RtmpGuesterImpl(callback);
}
void RTMPGuester::Destory(RTMPGuester*guester){
	RtmpGuesterImpl* impl = (RtmpGuesterImpl*)guester->GotSelfPtr();
	delete impl;
	guester = NULL;
}

RtmpGuesterImpl::RtmpGuesterImpl(RTMPGuesterEvent&callback)
	: callback_(callback)
	, worker_thread_(&webrtc::AnyRtmpCore::Inst())
	, av_rtmp_started_(NULL)
	, av_rtmp_player_(NULL)
	, m_pAudioMarker(nullptr)
	, video_render_(NULL){
	av_rtmp_player_ = AnyRtmplayer::Create(*this);
#ifdef _WIN32
	threadid = GetCurrentThreadId();
#endif
}


RtmpGuesterImpl::~RtmpGuesterImpl(){
#ifdef _WIN32
	assert(threadid == GetCurrentThreadId());
#endif
	StopRtmpPlay();
	if (av_rtmp_player_ != NULL) {
		//av_rtmp_player_->StopPlay();
		delete av_rtmp_player_;
		av_rtmp_player_ = NULL;
	}
	if (video_render_ != NULL) {
		delete video_render_;
		video_render_ = NULL;
	}
	if (m_pAudioMarker != nullptr) {
		delete [] m_pAudioMarker;
		m_pAudioMarker = nullptr;
	}
}

uint32_t RtmpGuesterImpl::SeekTo(uint32_t pos,double totaltime) {
#ifdef _WIN32
	assert(threadid == GetCurrentThreadId());
#endif
	if (av_rtmp_started_) {
		av_rtmp_player_->SeekTo(pos,totaltime);
	}
	return 0;
}

void RtmpGuesterImpl::StartRtmpPlay(const char* url, void* render, const char* sourcetype,const char* dir,int32_t enc, const char* userid, const short** pmarker,const int* lenmarker){
#ifdef _WIN32
	assert(threadid == GetCurrentThreadId());
#endif
	if(m_pAudioMarker != nullptr)
		delete[] m_pAudioMarker;
	int id = 0;
	m_iAudioMarker = 0;
	char idsub[10] = {0};
	for (int i = 0; i < strlen(userid); i++){
		memset(idsub, 0, sizeof(idsub));
		strncpy(idsub, &userid[i], 1);
		id = atoi(idsub);
		m_iAudioMarker += lenmarker[id] + 12000;
	}

	int RANGE_MIN = 200;
	int RANGE_MAX = 240;
	srand(time(NULL));
	m_iAudioMarketId = 0;
	m_iAudioMarketIdNew = 0;
	m_iAudioMarketLast = time(NULL);
	m_iAudioMarketStart[0] = (((double)rand() / (double)(RAND_MAX + 1)) * (RANGE_MAX - RANGE_MIN) + RANGE_MIN);
	m_iAudioMarketStart[0] -= 200;
	if (m_iAudioMarketStart[0] <= 20)
		m_iAudioMarketStart[0] = 20;
	for (int i = 1; i < sizeof(m_iAudioMarketStart)/sizeof(m_iAudioMarketStart[0]); i++){
		m_iAudioMarketStart[i] = m_iAudioMarketStart[i - 1] + m_iAudioMarketStart[i - 1] + (((double)rand() / (double)RAND_MAX) * RANGE_MAX + RANGE_MIN);
	}

	m_pAudioMarker = new char[m_iAudioMarker];
	memset(m_pAudioMarker, 0, m_iAudioMarker);
	m_pAudioMarketOut = (short*)m_pAudioMarker;
	memset(m_pAudioMarker, 0, m_iAudioMarker);
	short* pCur = (short*)m_pAudioMarker;
	for (int i = 0; i < strlen(userid); i++){
		memset(idsub, 0, sizeof(idsub));
		strncpy(idsub, &userid[i], 1);
		id = atoi(idsub);
		memcpy(pCur, pmarker[id], lenmarker[id]);
		pCur += lenmarker[id] / 2;
		memset(pCur, 0,12000);
		pCur += 12000 / 2;
		//short temp = *(pCur - 1);
		//for (int j = 0; j < 12000 / 2; j++) {
		//	(*pCur++) = temp;
		//}
	}

	if (!av_rtmp_started_) {
		av_rtmp_started_ = true;
		rtmp_url_ = url;
		video_render_ = webrtc::VideoRenderer::Create(render, 720, 80);
		av_rtmp_player_->SetVideoRender(video_render_);
		av_rtmp_player_->StartPlay(url,sourcetype,dir,enc);// , mabs);
		webrtc::AnyRtmpCore::Inst().StartAudioTrack(this);
	}
}

void RtmpGuesterImpl::StopRtmpPlay(){
#ifdef _WIN32
	assert(threadid == GetCurrentThreadId());
#endif
	if (av_rtmp_started_) {
		av_rtmp_started_ = false;
		rtmp_url_ = "";
		webrtc::AnyRtmpCore::Inst().StopAudioTrack();
		av_rtmp_player_->StopPlay();
		if (video_render_ != NULL) {
			delete video_render_;
			video_render_ = NULL;
		}
	}
}

//* For AnyRtmplayerEvent
void RtmpGuesterImpl::OnRtmplayerOK()
{
	callback_.OnRtmplayerOK();
}
void RtmpGuesterImpl::OnRtmplayerStatus(int cacheTime, int curBitrate,uint32_t curTime,double totalTime)
{
	callback_.OnRtmplayerStatus(cacheTime, curBitrate,curTime,totalTime);
}
void RtmpGuesterImpl::OnRtmplayerCache(int time)
{
	callback_.OnRtmplayerCache(time);
}
void RtmpGuesterImpl::OnRtmplayerClose(int errcode)
{
	callback_.OnRtmplayerClosed(errcode);
}

void RtmpGuesterImpl::OnGetPcmData(const void* p, const int len,const int type,const int)
{
	callback_.OnGetPcmData(p, len, type,0);
}
void RtmpGuesterImpl::OnRtmplayerPlayStart() {
	callback_.OnRtmplayerPlayStart();
}
void RtmpGuesterImpl::OnRtmplayerPlayStop() {
	callback_.OnRtmplayerPlayStop();
}
void RtmpGuesterImpl::OnRtmplayer1stVideo() {
	callback_.OnRtmplayer1stVideo();
}
void RtmpGuesterImpl::OnRtmplayer1stAudio() {
	callback_.OnRtmplayer1stAudio();
}

void RtmpGuesterImpl::OnRtmplayerConnectionFailed(int a) {
	callback_.OnRtmplayerConnectionFailed(a);
}

int RtmpGuesterImpl::OnNeedPlayAudio(void* audioSamples, uint32_t& samplesPerSec, size_t& channels){
	int ret = ((webrtc::AnyRtmplayerImpl*)av_rtmp_player_)->GetNeedPlayAudio(audioSamples, samplesPerSec, channels);
	//WCLOG(LS_ERROR) << "OnNeedPlayAudio" << ret << " sps:" << samplesPerSec <<" Channels:" << nChannels;
	while(ret > 0) {
		//CFile f;
		//if (f.Open(L"d:/test.pcm", CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite, NULL)) {
		//	f.SeekToEnd();
		//	f.Write(pcm, len);
		//	f.Close();
		//}
		if (false)
			break;
		short* p = (short*)audioSamples;
		if (m_pAudioMarketOut >= ((short*)m_pAudioMarker + m_iAudioMarker / 2)) {
			m_pAudioMarketOut = (short*)m_pAudioMarker;
		}
		for (int i = 0; i < ret / channels; i++) {
			short val = *m_pAudioMarketOut++;
			int adj = p[i];
			adj = p[i] * 0.96f + val;// ((p[i] * 0.7f) + val * 0.29f);
			if (adj > 32767) {
				adj = 32767;
			}
			else if (adj < -32767) {
				adj = -32767;
			}
			//}
			p[i] = adj;
			i++;
			p[i] = adj;
		}
		break;
	}
	return ret;
}