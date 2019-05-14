#pragma once

#include "AnyBaseSource.h"
#include <map>
#include <vector>
#include <string>
#include <list>


class AnyFlvSource :
	public AnyBaseSource{
public:
	AnyFlvSource();
	AnyFlvSource(const std::string,const std::string,int32_t );
	virtual ~AnyFlvSource();

	virtual int Create(std::string url); // 仿rtmp协议建立数据源接口。但是真实源可以是文件，rtmp,rtsp,等等已知文件或协议。
	virtual int Disconnect();	// 断开连接
	virtual int Handshake();	// 握手协议。
	virtual int Connectapp();	// 通知数据源连接app。live/livestream部分(rtmp://www.pic98.com/live/livestream)
	virtual int Playstream();	// 通知数据源开始获取数据。
	virtual uint32_t SeekTo(uint32_t,double);
	// 获取数据接口。参数全部参照rtmp协议。
	// type 数据类型。
	// timestamp 时间戳
	// data 数据缓冲
	// size 数据缓冲大小。
	virtual int Read(char* type, uint32_t* timestamp, char** data, int* size,TAG_HEADER& tag);
	virtual int Clear();
	virtual bool onMetaData(char type, char* data, int size);// { return true; };
	virtual bool NeedSlowdown() { return true; }
	virtual double GetPropDouble(const char* key) {
		double dbl = 0;
		std::map<std::string, double>::iterator iter = propdbl.find(key);
		if (iter != propdbl.end())
			dbl = iter->second;
		return dbl;
	};

private:
	std::string mfile;
	std::string mdir;
	std::vector<char> mbufv;
	int64_t mreadpos;
	int64_t mfirstreadlenmin;
	double mduration;
	char* mb;
	int32_t mencryption;
	int64_t mblen;
	uint32_t dwDataSizeLast;
	uint32_t mlasttimestamp;
	std::map<std::string, double> propdbl;
	std::map<std::string, std::string> propstr;
	std::map<std::string, bool> propbool;
};

