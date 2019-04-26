#include "AnyFlvSource.h"

// https://blog.csdn.net/sz76211822/article/details/53760836
typedef struct FLV_HEADER
{
	uint8_t btSignature[3];
	uint8_t btVersion;
	uint8_t btFlags;
	uint8_t btDataOffset[4];
	FLV_HEADER()
	{
		memset(this, 0, sizeof(FLV_HEADER));
	}
};

typedef struct TAG_HEADER
{
	uint8_t btPreviousTagSize[4];
	uint8_t btTagType;
	uint8_t btDataSize[3];
	uint8_t btTimeStamp[3];
	uint8_t btReserved;
	uint8_t btStreamID[3];
	TAG_HEADER()
	{
		memset(this, 0, sizeof(TAG_HEADER));
	}
};


AnyFlvSource::AnyFlvSource()
{
}

AnyFlvSource::AnyFlvSource(const std::string _file):
	mfile(_file),
	mbuf(nullptr),
	mbuftotallen(10240)
{
}

AnyFlvSource::~AnyFlvSource()
{
}


int AnyFlvSource::Create(std::string url) {
	//rtmp_ = srs_rtmp_create(url.c_str());
	//return rtmp_ == NULL ? 0 : 1;
	mbufcur = mbuf = new char[mbuftotallen];
	mbufleftlen = mbuftotallen;
	return 0;
}

int AnyFlvSource::Clear() {
	//if (rtmp_) {
	//	srs_rtmp_destroy(rtmp_);
	//	rtmp_ = NULL;
	//}
	delete [] mbuf;
	mbufcur = mbuf = nullptr;
	return 0;
}

int AnyFlvSource::Disconnect() {
	//if (rtmp_) {
	//	srs_rtmp_disconnect_server(rtmp_);
	//}
	return 0;
}

int AnyFlvSource::Handshake() {
	//if (rtmp_)
	//	return srs_rtmp_handshake(rtmp_);
	return 0;
}

int AnyFlvSource::Connectapp() {
	//if (rtmp_)
	//	return srs_rtmp_connect_app(rtmp_);
	mreaded = 0;
	return 0;
}

int AnyFlvSource::Playstream() {
	//if (rtmp_)
	//	return srs_rtmp_play_stream(rtmp_);
	mreaded = 0;
	return 0;
}

// 获取数据接口。参数全部参照rtmp协议。
// type 数据类型。
// timestamp 时间戳
// data 数据缓冲
// size 数据缓冲大小。
int AnyFlvSource::Read(char* type, uint32_t* timestamp, char** data, int* size) {
	*data = NULL;
	*size = 0;
	*timestamp = 0;
	*type = 0;
	int i = 0;
	FILE* f = _fsopen(mfile.c_str(), "rb", SH_DENYNO);
	if (f != nullptr) {
		if (mbuf == mbufcur) {
			mbufleftlen = mbuftotallen;
			int64_t readed = 0;
			fseek(f, 0, SEEK_SET);
			do {
				int64_t len = fread(mbufcur, 1, mbufleftlen, f);
				if (len <= 0)
					break;
				readed += len;
				mbufcur += len;
				mbufleftlen -= len;

			} while (readed >= 1024);
			if (readed >= 1024) {
				char s = 1;
				for (i = mfile.length() - 1; i >= 0; i--) {
					if (mfile[i] == '\\' || mfile[i] == '/') {
						s = atoi(mfile.substr(i + 1).c_str());
						break;
					}
				}
				
				if (s == 0) s = 1;
				for (int i = 0; i < 1024; i++) {
					mbuf[i] = mbuf[i] ^ s;
				}

			}
			else {
				return 0;
			}
			

		}
		fclose(f);
	}

	
	return 0;
}