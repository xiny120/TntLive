#include "AnyFlvSource.h"
#include "webrtc/base/logging.h"
#include "httpclient.h"



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
	mlasttimestamp = 0;
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
	mlasttimestamp = 0;
	return 0;
}

uint32_t AnyFlvSource::SeekTo(uint32_t pos,double totaltime) {
	int64_t lastmreadpos = mreadpos;
	std::vector<char> lastmbufv;
	int64_t lastmreadpos0 = 0;
	std::vector<char> lastmbufv0;
	uint32_t pos0;

	lastmbufv.assign(mbufv.begin(),mbufv.end());
	mreadpos = 0;
	mbufv.clear();
	int size;
	char type;
	char* data;
	uint32_t timestamp;
	uint32_t ret = 0;
	/**
	* E.4.1 FLV Tag, page 75
	*/
	// 8 = audio
//#define SRS_RTMP_TYPE_AUDIO 8
// 9 = video
//#define SRS_RTMP_TYPE_VIDEO 9
// 18 = script data
//#define SRS_RTMP_TYPE_SCRIPT 18
	while (true) { // SrsCodecVideoAVCFrame
		TAG_HEADER tag;
		int code = Read(&type, &timestamp, &data, &size,tag);
		if (code != 0)
			break;
		if (type == 0)
			break;
		
		uint32_t to = timestamp / 1000;
		if (data != NULL) {
			if (type == 9) {
				int8_t frame_type = data[0];
				frame_type = (frame_type >> 4) & 0x0f;
				if (frame_type == 1) {
					pos0 = to;
					lastmreadpos0 = mreadpos;
					lastmbufv0.clear();
					char* p = (char*)&tag;
					lastmbufv0.insert(lastmbufv0.end(),p, (p + sizeof(tag)));
					lastmbufv0.insert(lastmbufv0.end(), data, data + size);
					lastmbufv0.insert(lastmbufv0.end(),mbufv.begin(), mbufv.end());
					WCLOG(LS_ERROR) << "SeekTo Key Frame:" << pos << " (" << (int)frame_type << ") " << timestamp;
				}
			}
			free(data);
		}
		//WCLOG(LS_ERROR) << "SeekTo:" << pos << " to:" << to << " timestamp:" << timestamp ;
		if (to >= pos) {
			ret = pos0;
			mreadpos = lastmreadpos0;
			mbufv.clear();
			mbufv.assign(lastmbufv0.begin(), lastmbufv0.end());
			break;
		}
		if (to >= (totaltime - 20)) {
			WCLOG(LS_ERROR) << "SeekTo File End.";
			break;
		}
	}

	if (ret == 0) {
		mreadpos = lastmreadpos;
		mbufv.clear();
		mbufv.assign(lastmbufv.begin(), lastmbufv.end());
	}

	return ret;
}

// ��ȡ���ݽӿڡ�����ȫ������rtmpЭ�顣
// type �������͡�
// timestamp ʱ���
// data ���ݻ���
// size ���ݻ����С��
int AnyFlvSource::Read(char* type, uint32_t* timestamp, char** data, int* size,TAG_HEADER& tagout) {
	static std::thread::id idt = std::this_thread::get_id();
	if (idt != std::this_thread::get_id()) {
		WCLOG(LS_ERROR) << "�����߳�read:" << std::this_thread::get_id();
		idt = std::this_thread::get_id();
	}
	
	*data = NULL;
	*size = 0;
	*timestamp = 0;
	*type = 0;
	int i = 0;
	int64_t len = 0;
	FILE* f = nullptr;
	do {
		//FILE* f = _fsopen(mfile.c_str(), "rb", SH_DENYNO);
		FILE* f = fopen(mfile.c_str(), "rb");//, SH_DENYNO);
		if (f == nullptr) {
			WCLOG(LS_ERROR) << "_fsopen error:" << mfile << " error:" << errno;
			break;
		}
		do {
			fseek(f, mreadpos, SEEK_SET);
			if (mreadpos == 0) { // ��һ�ο�ʼ��ȡ�ļ�����һ����Ҫ��ȡ����1024�ֽڡ����ҵ�һ�ζ�ȡ�ļ���Ҫ����ͷ������ȡ����һ֡��Ч���ݡ�
				WCLOG(LS_ERROR) << "first time to read:" << mfile;
				len = 0;
				dwDataSizeLast = 0;
				fseek(f, mbufv.size(), SEEK_SET);
				do {
					len = fread(mb, 1, mblen, f);
					if (len <= 0)
						break;
					mbufv.insert(mbufv.end(), mb, mb + len);
					mreadpos = ftell(f);
					if (mbufv.size() >= mfirstreadlenmin)
						break;
				} while (true);
				if (len <= 0) {
					mreadpos = 0;
					mbufv.clear();
					WCLOG(LS_ERROR) << "first fread error:" << mfile << errno;
					break;
				}
				if (mbufv.size() < mfirstreadlenmin) { // û�ж����㹻���ݡ�
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
				if (len > mbufv.size()) { // ������̫С��
					mfirstreadlenmin = len;
					mreadpos = 0;
					mbufv.clear();
					WCLOG(LS_ERROR) << "first fread buffer not enough:" << mbufv.size() << "(need" << len << ")";
					break;
				}
				mbufv.erase(mbufv.begin(), mbufv.begin() + len);
			}
			if (mbufv.size() < sizeof(TAG_HEADER)) {
				len = 0;
				do {
					len = fread(mb, 1, mblen, f);
					if (len <= 0)
						break;
					mbufv.insert(mbufv.end(), mb, mb + len);
					mreadpos = ftell(f);
					if (mbufv.size() >= sizeof(TAG_HEADER))
						break;
				} while (true);
				if (len <= 0) {
					WCLOG(LS_ERROR) << "first fread error:" << mfile << errno;
					if ((mduration - 2) < mlasttimestamp / 1000) {

						return 0;
					}
					break;
				}
			}
			if (mbufv.size() < sizeof(TAG_HEADER)) { // û�ж����㹻���ݡ��ļ��������أ�û���ϲ��Ž��ȡ�
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
					mreadpos = ftell(f);
					if (mbufv.size() >= len)
						break;
				} while (true);
				if (readed <= 0) {
					//WCLOG(LS_ERROR) << "first fread error:" << mfile << errno;
					break;
				}
			}
			if (mbufv.size() < len) { // û�ж����㹻���ݡ��ļ��������أ�û���ϲ��Ž��ȡ�
				WCLOG(LS_ERROR) << "fread buffer not enough:" << mbufv.size() << "(need" << len << ")";
				break;
			}

			p0 = &mbufv[0];
			len = 0;
			ptag = (TAG_HEADER*)p0;
			memcpy(&tagout, ptag, sizeof(TAG_HEADER));
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
			mbufv.erase(mbufv.begin(), mbufv.begin() + len);
			//WCLOG(LS_ERROR) << "go frame  TotalSize:" << sizeof(TAG_HEADER) + dwDataSizeLast << " dwPreviousTagSize:" << dwPreviousTagSize << " dwDataSize:" << dwDataSize << " dwTimeStamp:" << dwTimeStamp;
			dwDataSizeLast = dwDataSize;
			mlasttimestamp = dwTimeStamp;
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
		case 0x02: // String type ����2���ֽ�Ϊ���� 
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
					if (key == "duration") {
						mduration = dbl;
					}
					WCLOG(LS_WARNING) << key << ":" << dbl << "(double)";
					break;
				case 0x01:
					b = *pcur++;
					WCLOG(LS_WARNING) << key << ":" << ((b != 0) ?"true":"false");
					propbool[key] = (b != 0) ? true : false;
					break;
				case 0x02: // String type ����2���ֽ�Ϊ���� 
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