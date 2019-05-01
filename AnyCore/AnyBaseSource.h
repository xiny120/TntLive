#pragma once
#include "LIV_Export.h"
#include <string>
#include <cstdint>

// https://blog.csdn.net/sz76211822/article/details/53760836
struct FLV_HEADER {
	uint8_t btSignature[3];
	uint8_t btVersion;
	uint8_t btFlags;
	uint8_t btDataOffset[4];
	FLV_HEADER() {
		memset(this, 0, sizeof(FLV_HEADER));
	}
};

struct TAG_HEADER {
	uint8_t btPreviousTagSize[4];
	uint8_t btTagType;
	uint8_t btDataSize[3];
	uint8_t btTimeStamp[3];
	uint8_t btReserved;
	uint8_t btStreamID[3];
	TAG_HEADER() {
		memset(this, 0, sizeof(TAG_HEADER));
	}
};

class LIV_API  AnyBaseSource
{
public:
	AnyBaseSource();
	virtual ~AnyBaseSource();
public:
	virtual int Create(std::string url) = 0; // 仿rtmp协议建立数据源接口。但是真实源可以是文件，rtmp,rtsp,等等已知文件或协议。
	virtual int Disconnect() = 0;	// 断开连接
	virtual int Handshake() = 0;	// 握手协议。
	virtual int Connectapp() = 0;	// 通知数据源连接app。live/livestream部分(rtmp://www.pic98.com/live/livestream)
	virtual int Playstream() = 0;	// 通知数据源开始获取数据。
	virtual int Clear() = 0;
	virtual bool onMetaData(char type, char* data, int size) = 0;
	virtual bool NeedSlowdown() { return false; }
	virtual uint32_t SeekTo(uint32_t,double) = 0;
	// 获取数据接口。参数全部参照rtmp协议。
	// type 数据类型。
	// timestamp 时间戳
	// data 数据缓冲
	// size 数据缓冲大小。
	virtual int Read(char* type, uint32_t* timestamp, char** data, int* size, TAG_HEADER& tag) = 0;
	virtual double GetPropDouble(const char* key) = 0;
};

