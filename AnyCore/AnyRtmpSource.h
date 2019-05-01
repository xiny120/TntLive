#pragma once
#include "LIV_Export.h"
#include "AnyBaseSource.h"


class LIV_API AnyRtmpSource :
	public AnyBaseSource
{
public:
	AnyRtmpSource();
	virtual ~AnyRtmpSource();
public:
	virtual int Create(std::string url) ; // 仿rtmp协议建立数据源接口。但是真实源可以是文件，rtmp,rtsp,等等已知文件或协议。
	virtual int Disconnect() ;	// 断开连接
	virtual int Handshake() ;	// 握手协议。
	virtual int Connectapp() ;	// 通知数据源连接app。live/livestream部分(rtmp://www.pic98.com/live/livestream)
	virtual int Playstream() ;	// 通知数据源开始获取数据。
	virtual uint32_t SeekTo(uint32_t, double) { return 0; };
	// 获取数据接口。参数全部参照rtmp协议。
	// type 数据类型。
	// timestamp 时间戳
	// data 数据缓冲
	// size 数据缓冲大小。
	virtual int Read(char* type, uint32_t* timestamp, char** data, int* size, TAG_HEADER& tag) ;
	virtual int Clear();
	virtual bool onMetaData(char type, char* data, int size);
	virtual double GetPropDouble(const char* key) { return 0.0f; };
protected:
	void* rtmp_;
};

