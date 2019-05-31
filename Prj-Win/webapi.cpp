#include "webapi.h"
#include <vector>
#include <map>
#include <list>
#include <sstream>

#ifdef _WIN32
#include <winsock.h>
#include <share.h>
#include <io.h>
#include <direct.h> 
#else
#include <unistd.h>
#include <sys/stat.h>
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


webapi::webapi(){
}


webapi::~webapi(){
}

webapi* webapi::me() {
	static webapi* wa = nullptr;
	if (wa == nullptr) {
		wa = new webapi();
	}
	return wa;
}

std::string UTF8_To_string(const std::string & str){
	int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t * pwBuf = new wchar_t[nwLen + 1];
	memset(pwBuf, 0, nwLen * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
	char * pBuf = new char[nLen + 1];
	memset(pBuf, 0, nLen + 1);
	WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
	std::string retStr = pBuf;
	delete[]pBuf;
	delete[]pwBuf;
	pBuf = NULL;
	pwBuf = NULL;
	return retStr;
}

std::string string_To_UTF8(const std::string & str){
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	wchar_t * pwBuf = new wchar_t[nwLen + 1];
	ZeroMemory(pwBuf, nwLen * 2 + 2);
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);
	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
	char * pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);
	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
	std::string retStr(pBuf);
	delete[]pwBuf;
	delete[]pBuf;
	pwBuf = NULL;
	pBuf = NULL;
	return retStr;
}


std::string webapi::post(char* url,char* sid, char* par) {
	std::string ret="";
	struct sockaddr_in stSockAddr;
	std::vector<char> vbuf;
	char buf[2048] = { 0 };
	urlitem ui;
	urlparse(url, ui);
	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	do {
		if (-1 == sockfd) {
			break;
		}
		int recvTimeout = 30 * 1000;  //30s
		int sendTimeout = 60 * 1000;  //30s
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&recvTimeout, sizeof(int));
		setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&sendTimeout, sizeof(int));
		memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
		stSockAddr.sin_family = AF_INET;
		stSockAddr.sin_port = htons(ui.port);
		stSockAddr.sin_addr.s_addr = inet_addr(ui.ip.front().c_str());
		do {
			if (-1 == connect(sockfd, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))) {
				//WCLOG(LS_ERROR) << "socket connect error:" << errno;
				break;
			}
			
			std::string post1 = prepareheader(&ui,sid,par);
			std::string post = post1;// string_To_UTF8(post1);
			OutputDebugStringA(post.c_str());
			int left = post.length();
			int length = post.length();
			int len, i, j;
			while (left > 0) {
				len = send(sockfd, post.c_str() + length - left, left, 0);
				
				if (len <= 0) {
					//WCLOG(LS_ERROR) << "socket send error:" << errno;
					break;
				}
				left -= len;
			}
			if (len <= 0)
				break;
			std::string header;
			i = 0;
			while (true) {
				len = recv(sockfd, buf, sizeof(buf), 0);
				if (len <= 0) {
					break;
				}
				vbuf.insert(vbuf.end(), buf, buf + len);
				if (vbuf.size() >= 4) {
					for (; i < vbuf.size() - 3; i++) {
						if (vbuf[i] == '\r' && vbuf[i + 1] == '\n' && vbuf[i + 2] == '\r' && vbuf[i + 3] == '\n') {
							header.append(&vbuf[0], i);
							//WCLOG(LS_ERROR) << "http header:" << header;
							vbuf.erase(vbuf.begin(), vbuf.begin() + i + 4);
							break;
						}
					}
				}
				if (!header.empty()) {
					break;
				}
			}

			if (len <= 0)
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
			if (httpvers.size() < 2 || !(httpvers[1] == "200" )) {
				break;
			}
			std::map<std::string, std::string>::iterator iter = headers.find("Content-Length");
			if (iter != headers.end()) {
				int contentlength = atoi((*iter).second.c_str());
				while (contentlength > vbuf.size()) {
					len = recv(sockfd, buf, sizeof(buf), 0);
					if (len <= 0) {
						break;
					}
					vbuf.insert(vbuf.end(), buf, buf + len);
				}
				//ret = UTF8_To_string(std::string(&vbuf[0],vbuf.size()));
				ret = std::string(&vbuf[0], vbuf.size());
			}


		} while (false);
	} while (false);
	return ret;
}

std::string webapi::prepareheader(const urlitem* ui,char* sid,std::string post) {
	std::ostringstream os;
	os << "POST " << ui->path << " HTTP/1.1\r\n"
		<< "Host:" << ui->host << "\r\n"
		<< "Content-Type:" << "application/json; charset=utf-8\r\n"
		<< "mster-token:" << sid << "\r\n"
		//<< "Connection:keep-alive\r\n"
		//<< "Upgrade-Insecure-Requests:1\r\n"
		<< "User-Agent:Mozilla/5.0(Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.25 Safari/537.36 Core/1.70.3676.400 QQBrowser/10.4.3457.400\r\n"
		<< "Accept:*/*\r\n"
		<< "Content-Length:" << post.size() << "\r\n"
		<< "Accept-Language: zh-CN,zh;q=0.9\r\n\r\n"
		<< post;
	return os.str();
}

bool webapi::urlparse(char* urlin1, urlitem & out) {
	bool ret = false;
	int i = 0;
	std::string urlin(urlin1);
	if (strcmp(urlin.substr(0, 4).c_str(), "http") != 0)
		return false;
	int lastpos = 0;
	int pos = 0;
	std::string childs[10];
	std::string temp;
	int idx = 0;
	out.fullpath = urlin;
	pos = urlin.find('?', 0);
	if (pos >= 0) {
		out.fullpath = urlin.substr(0, pos);
		out.query = urlin.substr(pos + 1);
	}
	//
	lastpos = 0;
	for (i = 0; i < out.query.length(); i++) {
		if (out.query[i] == '&') {
			temp = out.query.substr(lastpos, i - lastpos);
			pos = temp.find('=', 0);
			if (pos >= 0) {
				out.param[temp.substr(0, pos)] = temp.substr(pos + 1);
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
	}
	else if (pos = fullpath.find('\\') >= 0) {
		out.host = fullpath.substr(0, pos);
		out.path = fullpath.substr(pos);
	}
	pos = out.host.find(':');
	if (pos >= 0) {
		out.port = atoi(out.host.substr(pos + 1).c_str());
		out.host = out.host.substr(0, pos);
	}
	pos = out.path.rfind('/');
	if (pos >= 0) {
		out.file = out.path.substr(pos + 1);
	}
	else if (pos = out.path.rfind('\\') >= 0) {
		out.file = out.path.substr(pos + 1);
	}

	out.pathdir = out.path.substr(1, out.path.length() - out.file.length() - 1);

	struct hostent *hptr;
	char **pptr;
	if ((hptr = gethostbyname(out.host.c_str())) != nullptr) {
		if (hptr->h_name != nullptr)
			out.officalhost = hptr->h_name;
		for (pptr = hptr->h_aliases; *pptr != NULL; pptr++)
			out.aliases.push_back(*pptr);
		switch (hptr->h_addrtype) {
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