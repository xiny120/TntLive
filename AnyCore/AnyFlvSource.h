#pragma once
#include "AnyBaseSource.h"
class AnyFlvSource :
	public AnyBaseSource
{
public:
	AnyFlvSource();
	AnyFlvSource(const std::string);
	virtual ~AnyFlvSource();

	virtual int Create(std::string url); // 仿rtmp协议建立数据源接口。但是真实源可以是文件，rtmp,rtsp,等等已知文件或协议。
	virtual int Disconnect();	// 断开连接
	virtual int Handshake();	// 握手协议。
	virtual int Connectapp();	// 通知数据源连接app。live/livestream部分(rtmp://www.pic98.com/live/livestream)
	virtual int Playstream();	// 通知数据源开始获取数据。
	// 获取数据接口。参数全部参照rtmp协议。
	// type 数据类型。
	// timestamp 时间戳
	// data 数据缓冲
	// size 数据缓冲大小。
	virtual int Read(char* type, uint32_t* timestamp, char** data, int* size);
	virtual int Clear();

private:
	std::string mfile;
	int64_t mreaded;
	char* mbuf,*mbufcur;
	int64_t mbuftotallen;
	int64_t mbufleftlen;
	
};

