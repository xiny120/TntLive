#include "AnyFlvSource.h"
#include "webrtc/base/logging.h"

// https://blog.csdn.net/sz76211822/article/details/53760836
struct FLV_HEADER
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

struct TAG_HEADER
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
	mbuftotallen(10240000)
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
	mbegin = false;
	dwDataSizeLast = 0;
	return 0;
}

int AnyFlvSource::Clear() {
	//if (rtmp_) {
	//	srs_rtmp_destroy(rtmp_);
	//	rtmp_ = NULL;
	//}
	if (mbuf != nullptr) {
		delete[] mbuf;
		mbufcur = mbuf = nullptr;
	}
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
	mbegin = false;
	return 0;
}

int AnyFlvSource::Connectapp() {
	//if (rtmp_)
	//	return srs_rtmp_connect_app(rtmp_);
	mreaded = 0;
	mbegin = false;
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
		if (!mbegin && ( mbuf == mbufcur)) {
			mbegin = true;
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

			} while (true);// (readed < 1024);
			if (readed >= 1024) {
				char s = 1;
				for (i = mfile.length() - 1; i >= 0; i--) {
					if (mfile[i] == '\\' || mfile[i] == '/') {
						s = mfile.substr(i + 1)[5];// _atoi64(mfile.substr(i + 1).c_str());
						break;
					}
				}
				if (s == 1)
					s = mfile[5];// _atoi64(mfile.c_str());
				
				if (s == 0) s = 1;
				for (int i = 0; i < 1024; i++) {
					mbuf[i] = mbuf[i] ^ s;
				}

				FILE* fs = fopen("save.flv", "wb+");
				fwrite(mbuf, 1, readed, fs);
				fclose(fs);

				FLV_HEADER * pheader = (FLV_HEADER*)mbuf;
				int64_t len = mbufcur - mbuf;
				char* p0 = mbuf;
				p0 += sizeof(FLV_HEADER);
				TAG_HEADER* ptag = (TAG_HEADER*)p0;
				p0 += sizeof(TAG_HEADER);
				uint32_t dwPreviousTagSize = (ptag->btPreviousTagSize[0] << 24) | (ptag->btPreviousTagSize[1] << 16) | (ptag->btPreviousTagSize[2] << 8) | ptag->btPreviousTagSize[3];
				uint32_t dwDataSize = (ptag->btDataSize[0] << 16) | (ptag->btDataSize[1] << 8) | ptag->btDataSize[2];
				uint32_t dwTimeStamp = (ptag->btTimeStamp[0] << 16) | (ptag->btTimeStamp[1] << 8) | ptag->btTimeStamp[2];
				*data = NULL;
				*size = dwDataSize;
				*timestamp = dwTimeStamp;
				*type = ptag->btTagType;
				*data = new char[dwDataSize];
				memcpy(*data, p0, dwDataSize);
				p0 += dwDataSize;
				std::size_t left = mbufcur - p0;
				memcpy(mbuf, p0, left);
				mbufcur = mbuf + left;
				mbufleftlen = mbuftotallen - left;
				LOG(LS_ERROR) << "first frame dwPreviousTagSize:" << dwPreviousTagSize << " dwDataSize:" << dwDataSize << " dwTimeStamp:" << dwTimeStamp;
				dwDataSizeLast = dwDataSize;
			}
			else {
				mbufcur = mbuf;
				mbufleftlen = mbuftotallen;
				mbegin = false;
				return 0;
			}
		}else {
			if ((mbufcur - mbuf) < sizeof(TAG_HEADER)) {
				int64_t readed = 0;
				std::size_t left = 0;
				do {
					int64_t len = fread(mbufcur, 1, mbufleftlen, f);
					if (len <= 0)
						break;
					readed += len;
					mbufcur += len;
					mbufleftlen -= len;
					left = mbufcur - mbuf;

				} while (left < sizeof(TAG_HEADER));
			}
			if ((mbufcur - mbuf) >= sizeof(TAG_HEADER)) {
				char* p0 = mbuf;
				TAG_HEADER* ptag = (TAG_HEADER*)p0;
				p0 += sizeof(TAG_HEADER);
				uint32_t dwPreviousTagSize = (ptag->btPreviousTagSize[0] << 24) | (ptag->btPreviousTagSize[1] << 16) | (ptag->btPreviousTagSize[2] << 8) | ptag->btPreviousTagSize[3];
				uint32_t dwDataSize = (ptag->btDataSize[0] << 16) | (ptag->btDataSize[1] << 8) | ptag->btDataSize[2];
				uint32_t dwTimeStamp = (ptag->btTimeStamp[0] << 16) | (ptag->btTimeStamp[1] << 8) | ptag->btTimeStamp[2];
				*data = NULL;
				*size = dwDataSize;
				*timestamp = dwTimeStamp;
				*type = ptag->btTagType;

				LOG(LS_ERROR) << "go frame  TotalSize:" << sizeof(TAG_HEADER) + dwDataSizeLast << " dwPreviousTagSize:" << dwPreviousTagSize << " dwDataSize:" << dwDataSize << " dwTimeStamp:" << dwTimeStamp;
				dwDataSizeLast = dwDataSize;
				std::size_t left = mbufcur - p0;
				if (dwDataSize > left) {
					int64_t readed = 0;
					do {
						int64_t len = fread(mbufcur, 1, mbufleftlen, f);
						if (len <= 0)
							break;
						readed += len;
						mbufcur += len;
						mbufleftlen -= len;
						left = mbufcur - p0;

					} while (dwDataSize > left);
				}
				if (dwDataSize <= left) {
					*data = new char[dwDataSize];
					memcpy(*data, p0, dwDataSize);
					p0 += dwDataSize;
					std::size_t left = mbufcur - p0;
					memcpy(mbuf, p0, left);
					mbufcur = mbuf + left;
					mbufleftlen = mbuftotallen - left;
				}
				else {
					LOG(LS_ERROR) << "error";
					mbufcur = mbuf;
					mbufleftlen = mbuftotallen;
					mbegin = false;
					return 2;
				}

			}
		}
		fclose(f);
	}

	
	return 0;
}