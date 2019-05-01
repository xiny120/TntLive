#include "srs_librtmp.h"
#include "AnyRtmpSource.h"



AnyRtmpSource::AnyRtmpSource()
{
}

AnyRtmpSource::~AnyRtmpSource()
{

}

int AnyRtmpSource::Create(std::string url) {
	rtmp_ = srs_rtmp_create(url.c_str());
	return rtmp_ == NULL ? 0:1;
}

int AnyRtmpSource::Clear() {
	if (rtmp_) {
		srs_rtmp_destroy(rtmp_);
		rtmp_ = NULL;
	}
	return 0;
}

int AnyRtmpSource::Disconnect() {
	if (rtmp_) {
		srs_rtmp_disconnect_server(rtmp_);
	}
	return 0;
}

int AnyRtmpSource::Handshake() {
	if(rtmp_)
		return srs_rtmp_handshake(rtmp_);
	return 0;
}

int AnyRtmpSource::Connectapp() {
	if (rtmp_)
		return srs_rtmp_connect_app(rtmp_);
	return 0;
}

int AnyRtmpSource::Playstream() {
	if (rtmp_) {

		return srs_rtmp_play_stream(rtmp_);
	}
	return 0;
}

// 获取数据接口。参数全部参照rtmp协议。
// type 数据类型。
// timestamp 时间戳
// data 数据缓冲
// size 数据缓冲大小。
int AnyRtmpSource::Read(char* type, uint32_t* timestamp, char** data, int* size, TAG_HEADER& tag) {
	if (rtmp_)
		return srs_rtmp_read_packet(rtmp_, type, timestamp, data, size);
	return 0;
}

bool AnyRtmpSource::onMetaData(char type, char* data, int size) {
	return srs_rtmp_is_onMetaData(type, data, size);
}