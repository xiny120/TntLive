#include "httpclient.h"
#include <cstdio>
#include <sstream>
#include <iostream>
#include <locale>
#include <iomanip>

#ifdef WIN32
#include <io.h>
#include <direct.h> 
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

#ifdef _WIN32
#include <winsock.h>
#include <share.h>
#else
#include <sys/file.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>


#include <unistd.h>
#include <sys/uio.h>


#include <sys/types.h>
#include <errno.h>


#endif

#include "webrtc/base/logging.h"
//#include "../../../../../Users/root/AppData/Local/Android/Sdk/ndk-bundle/sources/cxx-stl/llvm-libc++/include/string"
//#include "../../../../../Users/root/AppData/Local/Android/Sdk/ndk-bundle/sources/cxx-stl/llvm-libc++/include/mutex"
//#include <utime.h>

#ifndef _WIN32
#define SOCKET int
#endif

#define MAX_PATH_LEN 256

#ifdef WIN32
#define ACCESS(fileName,accessMode) _access(fileName,accessMode)
#define MKDIR(path) _mkdir(path)
#else
#define ACCESS(fileName,accessMode) access(fileName,accessMode)
#define MKDIR(path) mkdir(path,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif


// 从左到右依次判断文件夹是否存在,不存在就创建
// example: /home/root/mkdir/1/2/3/4/
// 注意:最后一个如果是文件夹的话,需要加上 '\' 或者 '/'
int32_t createDirectory(const std::string &directoryPath){
	uint32_t dirPathLen = directoryPath.length();
	if (dirPathLen > MAX_PATH_LEN)	{
		return -1;
	}
	char tmpDirPath[MAX_PATH_LEN] = { 0 };
	for (uint32_t i = 0; i < dirPathLen; ++i)	{
		tmpDirPath[i] = directoryPath[i];
		if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')		{
			if (ACCESS(tmpDirPath, 0) != 0)		{
				int32_t ret = MKDIR(tmpDirPath);
				if (ret != 0){
					return ret;
				}
			}
		}
	}
	return 0;
}

NS_MONSTERLIVE_NET_BEGIN

httpclient::httpclient() :mrun(true)
{
}


httpclient::~httpclient()
{
}

void httpclient::run() {
	mpause = false;
	struct sockaddr_in stSockAddr;
	std::vector<char> vbuf;
	char buf[2048] = { 0 };
	while (mrun) {
		do {
			pullitem* pi = nullptr;
			{
				std::lock_guard<std::recursive_mutex> gs(this->sockmt);
				if (!_qpull.empty()) {
					pi = _qpull.front();
					if (pi->ui.firsttry == 0)
						pi->ui.firsttry = time(nullptr);
					int delay = (time(nullptr) - pi->ui.lastactive);
					if (delay < 5) {
						//WCLOG(LS_ERROR) << "retry delay:" << pi->filepeer << " delay:" << delay;
						
						pi = nullptr; // ���Ե�ʱ�򣬼��С��5�롣
					}
				}
			}
			if (pi == nullptr) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				break;
			}
			if (pi->ui.ip.size() <= 0) {
				WCLOG(LS_ERROR) << "dns error:" << pi->filepeer << " host:" << pi->ui.host << "no ip address";
				break;
			}
			if (mpause) {
				_qpull.pop_front();
				WCLOG(LS_ERROR) << "pause cmd clear queue:" << pi->filepeer;
				delete pi;
				break;
			}
			//std::string localfile = pi->filelocal;
			//if (localfile.empty()) {
			//	localfile = pi->ui.file;
			//}
			//std::string pathdir = pi->pathlocal + pi->ui.pathdir;
			std::string path = pi->filelocal;// localfile;
			//path = pathdir + localfile;
			//createDirectory(pathdir);
			std::string pathinfo = path + ".info";
			pi->ui.locallen = 0;
			pi->ui.lastmodify = getfiletime(pathinfo.c_str());
			struct stat sta;
			if (stat(path.c_str(), &sta) >= 0) {
				pi->ui.locallen = sta.st_size;
			}
			int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			pi->ui.active();
			if (-1 == sockfd) {
				WCLOG(LS_ERROR) << "socket create error:" << errno;
				break;
			}
			int recvTimeout = 30 * 1000;  //30s
			int sendTimeout = 60 * 1000;  //30s
			setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&recvTimeout, sizeof(int));
			setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&sendTimeout, sizeof(int));
			memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
			stSockAddr.sin_family = AF_INET;
			stSockAddr.sin_port = htons(pi->ui.port);
			stSockAddr.sin_addr.s_addr = inet_addr(pi->ui.ip.front().c_str());
			do {
				if (-1 == connect(sockfd, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))) {
					WCLOG(LS_ERROR) << "socket connect error:" << errno;
					break;
				}
				pi->ui.active();
				std::string post = prepareheader(&pi->ui);
				int left = post.length();
				int length = post.length();
				int len, i, j;
				while (left > 0) {
					len = send(sockfd, post.c_str() + length - left, left, 0);
					pi->ui.active();
					if (len <= 0) {
						WCLOG(LS_ERROR) << "socket send error:" << errno;
						break;
					}
					left -= len;
				}
				if (len <= 0) // ������������
					break;
				std::string header;
				i = 0;
				while (true) {
					len = recv(sockfd, buf, sizeof(buf), 0);
					pi->ui.active();
					if (len <= 0) {
						WCLOG(LS_ERROR) << "socket recv error:" << errno;
						break;
					}
					vbuf.insert(vbuf.end(), buf, buf + len);
					if (vbuf.size() >= 4) {
						for (; i < vbuf.size() - 3; i++) {
							if (vbuf[i] == '\r' && vbuf[i + 1] == '\n' && vbuf[i + 2] == '\r' && vbuf[i + 3] == '\n') {
								header.append(&vbuf[0], i);
								WCLOG(LS_ERROR) << "http header:" << header;
								vbuf.erase(vbuf.begin(), vbuf.begin() + i + 4);
								break;
							}
						}
					}
					if (!header.empty()) {
						break;
					}
				}
				if (len <= 0) // ������������
					break;
				header.append("\r\n");
				size_t pos = 0, lastpos = 0;
				std::string temp;
				std::map<std::string, std::string> headers; // Content-Disposition
				std::string httpver;
				for (i = 0; i < header.length() - 1; i++) {
					if (header[i] == '\r' && header[i + 1] == '\n') {
						temp = header.substr(lastpos, i - lastpos);
						lastpos = i + 2;
						for (j = 0; j < temp.length(); j++) {
							if (temp[j] == ':') {
								headers[temp.substr(0, j)] = temp.substr(j + 1);
								break;
							}
						}
						if (j == temp.length())
							httpver = temp;
					}
				}
				std::vector<std::string> httpvers;
				lastpos = 0;
				httpver.append(" ");
				for (i = 0; i < httpver.length(); i++) {
					if (httpver[i] == ' ') {
						std::string s0 = httpver.substr(lastpos, i - lastpos);
						s0.erase(std::find(s0.begin(), s0.end(), ' '));
						if (s0.length() > 0)
							httpvers.push_back(s0);
						lastpos = i;
					}
				}
				if (httpvers.size() < 2 || !(httpvers[1] == "200" || httpvers[1] == "206")) {
					WCLOG(LS_ERROR) << "http response error:" << httpver;
					std::lock_guard<std::recursive_mutex> gs(this->sockmt);
					_qpull.pop_front();
					delete pi;
					pi = nullptr;
					break;
				}
				std::map<std::string, std::string>::iterator iter = headers.find("Content-Length");
				if (iter != headers.end()) {
					pi->ui.contentlength = atoi((*iter).second.c_str());
				}
				pi->ui.totallen = 0;
				iter = headers.find("Content-Range");
				if (iter != headers.end()) {
					temp = (*iter).second;
					for (i = temp.length(); i >= 0; i--) {
						if (temp[i] == '/') {
							pi->ui.totallen = atoi(temp.substr(i + 1).c_str());
							break;
						}
					}
				}
				if (pi->ui.totallen == 0) {
					WCLOG(LS_ERROR) << "http response error: on Content-Range" << httpver;
					break;
				}
				bool samed = false;
				bool needremove = false;
				if (pi->ui.totallen == pi->ui.locallen) { // �ļ���С��ȡ��Ѿ����ع��ˡ��Ƚ��ļ����������Ƿ�һ����
					iter = headers.find("Last-Modified");
					if (str2time((*iter).second.c_str()) == pi->ui.lastmodify) { // ��С���޸�ʱ�䶼һ������Ϊ����ͬ�ļ�������Ҫ�ظ����ء�
						WCLOG(LS_ERROR) << "alerady have:" << pi->filelocal << " size:" << pi->ui.totallen << " Last Modify:" << pi->ui.lastmodify;
						std::lock_guard<std::recursive_mutex> gs(this->sockmt);
						_qpull.pop_front();
						delete pi;
						pi = nullptr;
						break;
					}else { // �ļ���Сһ�������Ƿ���˸��¹��ˡ�ɾ�������ļ���
						needremove = true;
					}
				}
				if (needremove || (pi->ui.totallen < pi->ui.locallen)) {
					i = 20;
					int removed = 0;
					do {
						removed = remove(pi->filelocal.c_str());
						if (removed == 0)
							break;
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
						if (mpause)
							break;
					} while (i-- >= 0);
					if(removed ==0)
						pi->ui.locallen = 0;
					else { // ��ǰ���̲���ɾ�������ļ�[��Ų��Ž�������ռ���ļ�]����Ҫ����ɾ����־�����´β���֮ǰɾ����
						WCLOG(LS_ERROR) << "file need to remove:" << pi->filelocal << " peer size:" << pi->ui.totallen << " local size:" << pi->ui.locallen;
						std::lock_guard<std::recursive_mutex> gs(this->sockmt);
						_qpull.pop_front();
						delete pi;
						pi = nullptr;
						break;
					}
				}
				WCLOG(LS_ERROR) << "start record file:" << pi->filelocal << " size:" << pi->ui.totallen << " Last Modify:" << pi->ui.lastmodify;
				//FILE* f = _fsopen(path.c_str(), "ab+", _SH_DENYWR);
                FILE* f = fopen(path.c_str(), "ab+");// _SH_DENYWR);
				if (f == nullptr) {
					WCLOG(LS_ERROR) << "downloading _fsopen error:" << path << errno;
					break;
				}
				while (!mpause) {
					len = recv(sockfd, buf, sizeof(buf), 0);
					//Sleep(100 * 2);
					pi->ui.active();
					if (len <= 0) {
						WCLOG(LS_ERROR) << "socket recv error:" << errno;
						break;
					}
					vbuf.insert(vbuf.end(), buf, buf + len);
					size_t writed = fwrite(&vbuf[0], 1, vbuf.size(), f);
					if (writed != vbuf.size()) {
						WCLOG(LS_ERROR) << "fwrite error:" << errno <<" writed:" << writed;
						pi->ui.locallen += writed;
						break;
					}
					pi->ui.locallen += vbuf.size();
					vbuf.clear();
					fflush(f);
					if (pi->ui.locallen >= pi->ui.totallen)
						break;
				}
				fclose(f);
				if (pi->ui.locallen >= pi->ui.totallen) {
					iter = headers.find("Last-Modified");
					setfiletime(pathinfo.c_str(), (*iter).second.c_str());
					WCLOG(LS_ERROR) << "file download finish:" << pi->filelocal << " filesize:" << pi->ui.locallen << "(" << pi->ui.totallen << ")";
					std::lock_guard<std::recursive_mutex> gs(this->sockmt);
					_qpull.pop_front();
					delete pi;
					break;
				}
				WCLOG(LS_ERROR) << "file download part one:" << pi->filelocal << " filesize:" << pi->ui.locallen << "(" << pi->ui.totallen << ")";
				std::lock_guard<std::recursive_mutex> gs(this->sockmt);
				_qpull.pop_front();
				pi->ui.failtimes++;
				_qpull.push_back(pi);

			} while (false);
#ifdef _WIN32
			closesocket(sockfd);
#else
			close(sockfd);
#endif
		}while (false);
		std::this_thread::sleep_for(std::chrono::milliseconds(0));
	}
}

httpclient* httpclient::me() {
	static httpclient* pThis = nullptr;
	if (pThis == nullptr) {
		pThis = new httpclient();
		if (pThis->Init() == 0) {
			return pThis;
		}
		delete pThis;
		pThis = nullptr;
	}
	return pThis;
}

int httpclient::Init() {
	sockthr = std::thread([this]() {this->run(); });
	return 0;
}

std::string httpclient::prepareheader(const urlitem* ui) {
	std::ostringstream os;
	os << "GET " << ui->url << " HTTP/1.1\r\n"
		<< "Host:" << ui->host << "\r\n"
		<< "Range:bytes=" << ui->locallen << " -\r\n"
		//<< "Connection:keep-alive\r\n"
		//<< "Upgrade-Insecure-Requests:1\r\n"
		<< "User-Agent:Mozilla/5.0(Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.25 Safari/537.36 Core/1.70.3676.400 QQBrowser/10.4.3457.400\r\n"
		<< "Accept:*/*\r\n"
		//<< "Accept-Encoding: gzip, deflate\r\n"
		<< "Accept-Language: zh-CN,zh;q=0.9\r\n\r\n";
	return os.str();
}

bool httpclient::urlparse(std::string urlin, urlitem & out) {
	bool ret = false;
	int i = 0;
	if (strcmp(urlin.substr(0, 4).c_str(), "http") != 0)
		return false;
	int lastpos = 0;
	int pos = 0;
	std::string childs[10];
	std::string temp;
	int idx = 0;
	out.url = urlin;
	// ���ҡ�?��
	out.fullpath = urlin;
	pos = urlin.find('?', 0);
	if (pos >= 0) {
		out.fullpath = urlin.substr(0, pos);
		out.query = urlin.substr(pos+1);
	}
	// ����query�� & �ָ�����ԣ���=�ָ� k-v
	lastpos = 0;
	for (i = 0; i < out.query.length(); i++) {
		if (out.query[i] == '&') {
			temp = out.query.substr(lastpos, i - lastpos);
			pos = temp.find('=', 0);
			if (pos >= 0) {
				out.param[temp.substr(0, pos)] = temp.substr(pos+1);
			}
			lastpos = i + 1;
		}
	}
	temp = out.query.substr(lastpos);
	pos = temp.find('=', 0);
	if (pos >= 0) {
		out.param[temp.substr(0, pos)] = temp.substr(pos + 1);
	}
	lastpos = 0;
	idx = 0;
	for (i = 0; i < out.fullpath.length(); i++) {
		if (out.fullpath[i] == ':') {
			childs[idx] = out.fullpath.substr(lastpos, i - lastpos);
			lastpos = i + 1;
			idx++;
			break;
		}
	}
	out.prot = childs[0];
	out.port = 80;
	std::string fullpath = out.fullpath.substr(out.prot.length() + 3);
	pos = fullpath.find('/');
	if (pos >= 0) {
		out.host = fullpath.substr(0, pos);
		out.path = fullpath.substr(pos);
	}else if (pos = fullpath.find('\\') >= 0) {
		out.host = fullpath.substr(0, pos);
		out.path = fullpath.substr(pos);
	}
	pos = out.host.find(':');
	if (pos >= 0) {
		out.port = atoi(out.host.substr(pos+1).c_str());
		out.host = out.host.substr(0, pos);
	}
	pos = out.path.rfind('/');
	if (pos >= 0) {
		out.file = out.path.substr(pos + 1);
	}else if (pos = out.path.rfind('\\') >= 0) {
		out.file = out.path.substr(pos + 1);
	}

	out.pathdir = out.path.substr(1,out.path.length() - out.file.length()-1);

	struct hostent *hptr;
	char **pptr;
	if ((hptr = gethostbyname(out.host.c_str())) != nullptr) {
		if (hptr->h_name != nullptr)
			out.officalhost = hptr->h_name;
		for (pptr = hptr->h_aliases; *pptr != NULL; pptr++)
			out.aliases.push_back(*pptr);
		switch (hptr->h_addrtype){
		case AF_INET:
		//case AF_INET6:
			pptr = hptr->h_addr_list;
			for (; *pptr != NULL; pptr++) {
				out.ip.push_back(inet_ntoa(*((in_addr *)*pptr)));
			}
			break;
		default:
			break;
		}
	}
	return true;
}

// url Ҫ���ص��ļ���url, localfile ���غ󱾵ر�����ļ��������Ϊ�գ����ؾͷ���url�е��ļ����� localpath,���ر����Ŀ¼��
bool httpclient::get(const std::string url, std::string& localfile_,const std::string localpath) {
	mpause = false;
	bool ret = false;
	pullitem *pi = new pullitem();
	pi->filepeer = url;
	pi->filelocal = localfile_.c_str();
	pi->pathlocal = localpath.c_str();
	localfile_ = "shit";
	ret = urlparse(url, pi->ui);
	if (ret) {
		if (pi->filelocal.length() < 1) {
			std::string lf = pi->filelocal.c_str();
			if (lf.empty()) {
				lf = pi->ui.file.c_str();
			}
			std::string pathdir = pi->pathlocal + "/" + pi->ui.pathdir;
			std::string path = lf;
			path = pathdir +"/"+ lf;
			createDirectory(pathdir);

			pi->filelocal = path;
			localfile_ = path;
		}
		std::lock_guard<std::recursive_mutex> gs(this->sockmt);
		listpull::iterator iter;
		for (iter = _qpull.begin(); iter != _qpull.end(); iter++) {
			if ((*iter)->filelocal.compare(pi->filelocal) == 0)
				break;
		}
		if(iter == _qpull.end())
			_qpull.push_back(pi);
		//_qpull.push(pi);
		start();
	}
	else {
		delete pi;
	}
	return ret;
}



bool httpclient::isfileexists(const char*path) {
	struct stat sta;
	if (stat(path, &sta) >= 0) {
		return true;
	}
	return false;
}

time_t httpclient::str2time(const char* str) {
	//Fri, 22 Mar 2019 02:06 : 06 GMT
	std::tm t = {};
	std::istringstream ss(str);
	ss >> std::get_time(&t, "%a, %d %b %Y %H:%M:%S");
	if (!ss.fail()) {
		return mktime(&t);
	}
	return 0;
}

time_t httpclient::getfiletime(const char* file) {
	time_t  t = 0;
	FILE* f = fopen(file, "rb");
	if (f != nullptr) {
		if (fread(&t, sizeof(t), 1, f) > 0) {
		}
		fclose(f);
	}
	return t;
}

void httpclient::setfiletime(const char* file, const char* time) {
	time_t t = str2time(time);
	FILE* f = fopen(file, "wb");
	if (f != nullptr) {
		if (fwrite(&t,sizeof(t) , 1, f) > 0) {
		}
		fclose(f);
	}
}


NS_MONSTERLIVE_NET_END