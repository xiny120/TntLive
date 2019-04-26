#include "httpclient.h"
#include <sstream>
#include <iostream>
#include <locale>
#include <iomanip>
#include <winsock.h>
#include <share.h>
//#include <utime.h>

NS_MONSTERLIVE_NET_BEGIN

httpclient::httpclient() :mrun(true)
{
}


httpclient::~httpclient()
{
}

void httpclient::run() {
	mpause = false;
	while (mrun) {
		pullitem* pi = nullptr;
		{
			std::lock_guard<std::recursive_mutex> gs(this->sockmt);
			if (!_qpull.empty()) {
				pi = _qpull.front();
				if (pi->ui.firsttry == 0)
					pi->ui.firsttry = time(nullptr);
				if ((time(nullptr) - pi->ui.lastactive) < 5) {
					pi = nullptr; // 重试的时候，间隔小于5秒。
				}
				//_qpull.pop();
			}
		}
		if (pi != nullptr && pi->ui.ip.size() > 0) {
			struct sockaddr_in stSockAddr;

			std::string localfile = pi->filelocal;
			if (localfile.empty()) {
				localfile = pi->ui.file;
			}
			std::string path = localfile;
			if (!pi->pathlocal.empty()) {
				path = pi->pathlocal + "/" + localfile;
			}
			std::string pathlock = path;// +".downloading";
			std::string pathinfo = path + ".info";
			pi->ui.locallen = 0;
			pi->ui.lastmodify = getfiletime(pathinfo.c_str());
			struct stat sta;
			if (stat(pathlock.c_str(), &sta) >= 0) {
				pi->ui.locallen = sta.st_size;
			}else if(stat(path.c_str(),&sta) >=0) {
				pi->ui.locallen = sta.st_size;
			}

			int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			pi->ui.active();
			if (-1 != sockfd) {
				memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
				stSockAddr.sin_family = AF_INET;
				stSockAddr.sin_port = htons(pi->ui.port);
				stSockAddr.sin_addr.S_un.S_addr = inet_addr(pi->ui.ip.front().c_str());
				do{
					if (-1 != connect(sockfd, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
					{
						pi->ui.active();
						std::string header = prepareheader(&pi->ui);
						int left = header.length();
						int length = header.length();
						int len, i, j;
						while (left > 0) {
							len = send(sockfd, header.c_str() + length - left, left, 0);
							pi->ui.active();
							if (len <= 0)
								break;
							left -= len;
						}
						if (left == 0) {
							char buf[1024] = { 0 };
							char bufleft[1024] = { 0 };
							int bufleftlen = 0;
							int headerlen = 0;
							bool headerdone = false;
							std::string header;
							while (true) {
								len = recv(sockfd, buf, sizeof(buf), 0);
								pi->ui.active();
								if (len <= 0)
									break;
								for (i = 0; i < len - 3; i++) {
									if (buf[i] == '\r' && buf[i + 1] == '\n' && buf[i + 2] == '\r' && buf[i + 3] == '\n') {
										headerlen += i;
										bufleftlen = len - (i + 4);
										memcpy(bufleft, buf + i + 4, bufleftlen);
										len = i;
										headerdone = true;
										break;
									}
								}
								header.append(buf, len);
								if (headerdone)
									break;
							}

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
								std::lock_guard<std::recursive_mutex> gs(this->sockmt);
								_qpull.pop();
								break;
							}
							// Content-Length Last-Modified
							std::map<std::string, std::string>::iterator iter = headers.find("Content-Length");
							bool samed = false;
							if (iter != headers.end()) {
								pi->ui.totallen = atoi((*iter).second.c_str());
								if (pi->ui.totallen == pi->ui.locallen) {
									iter = headers.find("Last-Modified");
									if (str2time((*iter).second.c_str()) == pi->ui.lastmodify) {
										samed = true;
									}
								}
								else if (pi->ui.totallen < pi->ui.locallen) {
									remove(pi->filelocal.c_str());
									pi->ui.locallen = 0;
								}
							}

							if (samed) { // 已经下载了，并且大小和修改时间一致。不需要下载了。
								std::lock_guard<std::recursive_mutex> gs(this->sockmt);
								_qpull.pop();
							}else {
								//remove(pathlock.c_str());
								FILE* f = _fsopen(pathlock.c_str(), "ab+", _SH_DENYWR);
								if (f != nullptr) {
									if (fwrite(bufleft, 1, bufleftlen, f) >= 0) {
										pi->ui.locallen += bufleftlen;
										while (!mpause) {
											len = recv(sockfd, buf, sizeof(buf), 0);
											pi->ui.active();
											if (len <= 0)
												break;
											if (fwrite(buf, 1, len, f) < 0)
												break;
											fflush(f);
											pi->ui.locallen += len;
											if (pi->ui.locallen == pi->ui.totallen)
												break;
										}
									}
									fclose(f);
									if (pi->ui.locallen == pi->ui.totallen) {
										//rename(pathlock.c_str(), path.c_str());
										iter = headers.find("Last-Modified");
										//time_t tmodify = str2time((*iter).second.c_str());
										setfiletime(pathinfo.c_str(), (*iter).second.c_str());
										if (!mpause) { // 不是暂停，正常下载完毕。
											std::lock_guard<std::recursive_mutex> gs(this->sockmt);
											_qpull.pop();
										}
									}
								}
							}
						}
					}
				}while (false);
				closesocket(sockfd);
			}
		}
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
		<< "Ranges:bytes=" << ui->locallen << " -\r\n"
		<< "Connection:keep-alive\r\n"
		<< "Upgrade-Insecure-Requests:1\r\n"
		<< "User-Agent:Mozilla/5.0(Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.25 Safari/537.36 Core/1.70.3676.400 QQBrowser/10.4.3457.400\r\n"
		<< "Accept:text/html, application/xhtml+xml, application/xml; q=0.9,image/webp,image/apng,*/*;q=0.8\r\n"
		<< "Accept-Encoding: gzip, deflate\r\n"
		<< "Accept-Language: zh-CN,zh;q=0.9\r\n\r\n";


	return os.str();

}

bool httpclient::urlparse(std::string urlin, urlitem & out) {
	bool ret = false;
	int i = 0;
	if (_stricmp(urlin.substr(0, 4).c_str(), "http") != 0)
		return false;
	int lastpos = 0;
	int pos = 0;
	std::string childs[10];
	std::string temp;
	int idx = 0;
	out.url = urlin;
	// 查找‘?’
	out.fullpath = urlin;
	pos = urlin.find('?', 0);
	if (pos >= 0) {
		out.fullpath = urlin.substr(0, pos);
		out.query = urlin.substr(pos+1);
	}
	// 分析query以 & 分割参数对，以=分割 k-v
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

// url 要下载的文件的url, localfile 下载后本地保存的文件名，如果为空，返回就返回url中的文件名。 localpath,本地保存的目录。
bool httpclient::get(std::string url, std::string& localfile,const std::string& localpath) {
	mpause = false;
	bool ret = false;
	pullitem *pi = new pullitem();
	pi->filepeer = url;
	pi->filelocal = localfile;
	pi->pathlocal = localpath;
	ret = urlparse(url, pi->ui);
	if (ret) {
		if (pi->filelocal.empty()) {
			pi->filelocal = pi->ui.file;
			localfile = pi->ui.file;
		}
		std::lock_guard<std::recursive_mutex> gs(this->sockmt);
		_qpull.push(pi);
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