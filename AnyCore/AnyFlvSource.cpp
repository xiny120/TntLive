#include "AnyFlvSource.h"
#include "webrtc/base/logging.h"
#include "httpclient.h"

// https://blog.csdn.net/sz76211822/article/details/53760836
struct FLV_HEADER{
	uint8_t btSignature[3];
	uint8_t btVersion;
	uint8_t btFlags;
	uint8_t btDataOffset[4];
	FLV_HEADER(){
		memset(this, 0, sizeof(FLV_HEADER));
	}
};

struct TAG_HEADER{
	uint8_t btPreviousTagSize[4];
	uint8_t btTagType;
	uint8_t btDataSize[3];
	uint8_t btTimeStamp[3];
	uint8_t btReserved;
	uint8_t btStreamID[3];
	TAG_HEADER(){
		memset(this, 0, sizeof(TAG_HEADER));
	}
};

AnyFlvSource::AnyFlvSource(){
}

AnyFlvSource::AnyFlvSource(const std::string _file):
	mfile(_file),
	mb(nullptr),
	mblen(102400){
}

AnyFlvSource::~AnyFlvSource(){
	monsterlive::net::httpclient::me()->pause();
}

int AnyFlvSource::Create(std::string url) {
	mb = new char[mblen];
	dwDataSizeLast = 0;
	mreadpos = 0;
	mbufv.clear();
	mbufv.reserve(mblen);
	return 0;
}

int AnyFlvSource::Clear() {
	if (mb != nullptr) {
		delete[] mb;
		mb = nullptr;
	}
	mbufv.clear();
	mreadpos = 0;
	return 0;
}

int AnyFlvSource::Disconnect() {
	return 0;
}

int AnyFlvSource::Handshake() {
	mreadpos = 0;
	return 0;
}

int AnyFlvSource::Connectapp() {
	mreadpos = 0;
	return 0;
}

int AnyFlvSource::Playstream() {
	mreadpos = 0;
	dwDataSizeLast = 0;
	mfirstreadlenmin = 1024;
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
	int64_t len = 0;
	FILE* f = nullptr;
	do {
		FILE* f = _fsopen(mfile.c_str(), "rb", SH_DENYNO);
		if (f == nullptr) {
			WCLOG(LS_ERROR) << "_fsopen error:" << mfile << " error:" << errno;
			break;
		}
		do {
			fseek(f, mreadpos, SEEK_SET);
			if (mreadpos == 0) { // 第一次开始读取文件，第一次需要读取最少1024字节。并且第一次读取文件需要分析头，并读取出第一帧有效数据。
				WCLOG(LS_ERROR) << "first time to read:" << mfile;
				len = 0;
				dwDataSizeLast = 0;
				fseek(f, mbufv.size(), SEEK_SET);
				do {
					len = fread(mb, 1, mblen, f);
					if (len <= 0)
						break;
					mbufv.insert(mbufv.end(), mb, mb + len);
					mreadpos = _ftelli64(f);
					if (mbufv.size() >= mfirstreadlenmin)
						break;
				} while (true);
				if (len <= 0) {
					mreadpos = 0;
					mbufv.clear();
					WCLOG(LS_ERROR) << "first fread error:" << mfile << errno;
					break;
				}
				if (mbufv.size() < mfirstreadlenmin) { // 没有读到足够数据。
					mreadpos = 0;
					mbufv.clear();
					WCLOG(LS_ERROR) << "first fread buffer not enough:" << mbufv.size() << "(need 1024)";
					break;
				}
				char s = 1;
				for (i = mfile.length() - 1; i >= 0; i--) {
					if (mfile[i] == '\\' || mfile[i] == '/') {
						s = mfile.substr(i + 1)[5];
						break;
					}
				}
				if (s == 1)
					s = mfile[5];
				if (s == 0) s = 1;
				for (i = 0; i < 1024; i++) {
					mbufv[i] = mbufv[i] ^ s;
				}
				len = 0;
				char* p0 = &mbufv[0];
				FLV_HEADER* ph = (FLV_HEADER*)p0;
				p0 += sizeof(FLV_HEADER);
				len += sizeof(FLV_HEADER);
				if (len > mbufv.size()) {
					mfirstreadlenmin = len;
					mreadpos = 0;
					mbufv.clear();
					WCLOG(LS_ERROR) << "first fread buffer not enough:" << mbufv.size() << "(need" << len << ")";
					break;
				}
				TAG_HEADER* ptag = (TAG_HEADER*)p0;
				p0 += sizeof(TAG_HEADER);
				len += sizeof(TAG_HEADER);
				if (len > mbufv.size()) {
					mfirstreadlenmin = len;
					mreadpos = 0;
					mbufv.clear();
					WCLOG(LS_ERROR) << "first fread buffer not enough:" << mbufv.size() << "(need" << len << ")";
					break;
				}
				uint32_t dwPreviousTagSize = (ptag->btPreviousTagSize[0] << 24) | (ptag->btPreviousTagSize[1] << 16) | (ptag->btPreviousTagSize[2] << 8) | ptag->btPreviousTagSize[3];
				uint32_t dwDataSize = (ptag->btDataSize[0] << 16) | (ptag->btDataSize[1] << 8) | ptag->btDataSize[2];
				uint32_t dwTimeStamp = (ptag->btTimeStamp[0] << 16) | (ptag->btTimeStamp[1] << 8) | ptag->btTimeStamp[2];
				len += dwDataSize;
				if (len > mbufv.size()) {
					mfirstreadlenmin = len;
					mreadpos = 0;
					mbufv.clear();
					WCLOG(LS_ERROR) << "first fread buffer not enough:" << mbufv.size() << "(need" << len << ")";
					break;
				}
				*size = dwDataSize;
				*timestamp = dwTimeStamp;
				*type = ptag->btTagType;
				*data = new char[dwDataSize];
				memcpy(*data, p0, dwDataSize);
				p0 += dwDataSize;
				mbufv.erase(mbufv.begin(),mbufv.begin() + len);
				break;
			}
			if (mbufv.size() < sizeof(TAG_HEADER)) {
				len = 0;
				do {
					len = fread(mb, 1, mblen, f);
					if (len <= 0)
						break;
					mbufv.insert(mbufv.end(), mb, mb + len);
					mreadpos = _ftelli64(f);
					if (mbufv.size() >= sizeof(TAG_HEADER))
						break;
				} while (true);
				if (len <= 0) {
					WCLOG(LS_ERROR) << "first fread error:" << mfile << errno;
					break;
				}
			}
			if (mbufv.size() < sizeof(TAG_HEADER)) { // 没有读到足够数据。文件正在下载，没赶上播放进度。
				WCLOG(LS_ERROR) << "fread buffer not enough:" << mbufv.size() << "(need" << sizeof(TAG_HEADER) << ")";
				break;
			}
			char* p0 = &mbufv[0];
			len = 0;
			TAG_HEADER* ptag = (TAG_HEADER*)p0;
			p0 += sizeof(TAG_HEADER);
			len += sizeof(TAG_HEADER);
			uint32_t dwPreviousTagSize = (ptag->btPreviousTagSize[0] << 24) | (ptag->btPreviousTagSize[1] << 16) | (ptag->btPreviousTagSize[2] << 8) | ptag->btPreviousTagSize[3];
			uint32_t dwDataSize = (ptag->btDataSize[0] << 16) | (ptag->btDataSize[1] << 8) | ptag->btDataSize[2];
			uint32_t dwTimeStamp = (ptag->btTimeStamp[0] << 16) | (ptag->btTimeStamp[1] << 8) | ptag->btTimeStamp[2];

			len += dwDataSize;
			if (mbufv.size() < len) {
				int readed = 0;
				do {
					readed = fread(mb, 1, mblen, f);
					if (readed <= 0)
						break;
					mbufv.insert(mbufv.end(), mb, mb + readed);
					mreadpos = _ftelli64(f);
					if (mbufv.size() >= len)
						break;
				} while (true);
				if (readed <= 0) {
					//WCLOG(LS_ERROR) << "first fread error:" << mfile << errno;
					break;
				}
			}
			if (mbufv.size() < len) { // 没有读到足够数据。文件正在下载，没赶上播放进度。
				WCLOG(LS_ERROR) << "fread buffer not enough:" << mbufv.size() << "(need" << len << ")";
				break;
			}

			p0 = &mbufv[0];
			len = 0;
			ptag = (TAG_HEADER*)p0;
			p0 += sizeof(TAG_HEADER);
			len += sizeof(TAG_HEADER);
			dwPreviousTagSize = (ptag->btPreviousTagSize[0] << 24) | (ptag->btPreviousTagSize[1] << 16) | (ptag->btPreviousTagSize[2] << 8) | ptag->btPreviousTagSize[3];
			dwDataSize = (ptag->btDataSize[0] << 16) | (ptag->btDataSize[1] << 8) | ptag->btDataSize[2];
			dwTimeStamp = (ptag->btTimeStamp[0] << 16) | (ptag->btTimeStamp[1] << 8) | ptag->btTimeStamp[2];
			*size = dwDataSize;
			*timestamp = dwTimeStamp;
			*type = ptag->btTagType;
			*data = new char[dwDataSize];
			memcpy(*data, p0, dwDataSize);
			p0 += dwDataSize;
			len += dwDataSize;
			//mreadpos = mbufv.size();
			mbufv.erase(mbufv.begin(), mbufv.begin() + len);
			//WCLOG(LS_ERROR) << "go frame  TotalSize:" << sizeof(TAG_HEADER) + dwDataSizeLast << " dwPreviousTagSize:" << dwPreviousTagSize << " dwDataSize:" << dwDataSize << " dwTimeStamp:" << dwTimeStamp;
			dwDataSizeLast = dwDataSize;

		} while(false);
		fclose(f);
	} while (false);
	return 0;
}

bool AnyFlvSource::onMetaData(char type_, char* data, int size) { 
	bool end = false;
	char* pcur = data;
	int i = 0;
	uint8_t b = 0;
	uint16_t len = 0;
	uint32_t arrlen = 0;
	std::string key,val;
	char type = 0;
	double dbl = 0;
	while ((pcur - data) < size) {
		type = *pcur;
		pcur++;
		switch (type)
		{
		case 0x00: // Number type 8 Bypte Double 
			pcur += 8;
			break;
		case 0x01: // Boolean type 1 Bypte bool 
			pcur += 1;
			break;
		case 0x02: // String type 后面2个字节为长度 
			len = *pcur;
			pcur++;
			len = len << 8 | *pcur;
			pcur++;
			val.assign(pcur, len);
			break;
		case 0x03:
			end = false;
			while (!end && (pcur - data) < size) {
				len = *pcur;
				pcur++;
				len = len << 8 | *pcur;
				pcur++;
				key.assign(pcur, len);
				pcur += len;
				type = *pcur;
				pcur++;
				switch (type)
				{
				case 0x00:
					for (i = 0; i < 8; i++) {
						((char*)&dbl)[7-i] = *pcur++;
					}
					propdbl[key] = dbl;
					WCLOG(LS_WARNING) << key << ":" << dbl << "(double)";
					break;
				case 0x01:
					b = *pcur++;
					WCLOG(LS_WARNING) << key << ":" << ((b != 0) ?"true":"false");
					propbool[key] = (b != 0) ? true : false;
					break;
				case 0x02: // String type 后面2个字节为长度 
					len = *pcur;
					pcur++;
					len = len << 8 | *pcur;
					pcur++;
					val.assign(pcur, len);
					pcur += len;
					WCLOG(LS_WARNING) << key << ":" << val;
					propstr[key] = val;
					break;
				case 0x09:
					end = true;
					break;
				default:
					printf("");
					break;
				}
			}
			break;
		case 0x04:
			break;
		case 0x05:
			break;
		case 0x06:
		case 0x07:
			break;
		case 0x08:
			arrlen = *pcur;
			pcur++;
			arrlen = arrlen << 8 | *pcur;
			pcur++;
			arrlen = arrlen << 16 | *pcur;
			pcur++;
			arrlen = arrlen << 24 | *pcur;
			pcur++;
			break;
		default:
			break;
		}
		
	}
	return true; 
}