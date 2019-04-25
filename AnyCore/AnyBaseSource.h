#pragma once
#include "LIV_Export.h"
#include <string>
#include <cstdint>
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
	// 获取数据接口。参数全部参照rtmp协议。
	// type 数据类型。
	// timestamp 时间戳
	// data 数据缓冲
	// size 数据缓冲大小。
	virtual int Read(char* type, uint32_t* timestamp, char** data, int* size) = 0; 
};

