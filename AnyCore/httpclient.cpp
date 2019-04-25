#include "httpclient.h"
#include <sstream>
#include <winsock.h>

NS_MONSTERLIVE_NET_BEGIN

httpclient::httpclient():mrun(true)
{
}


httpclient::~httpclient()
{
}

void httpclient::run() {
	while (mrun) {
		pullitem* pi = nullptr;
		{
			std::lock_guard<std::recursive_mutex> gs(this->sockmt);
			if (!_qpull.empty()) {
				pi = _qpull.front();
				_qpull.pop();
			}
		}
		if (pi != nullptr && pi->ui.ip.size() > 0) {
			struct sockaddr_in stSockAddr;
			int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (-1 != sockfd) {
				memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
				stSockAddr.sin_family = AF_INET;
				stSockAddr.sin_port = htons(pi->ui.port);
				stSockAddr.sin_addr.S_un.S_addr = inet_addr(pi->ui.ip.front().c_str());
				if (-1 != connect(sockfd, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
				{
					std::string header = prepareheader(&pi->ui);
					int left = header.length();
					int length = header.length();
					while (left > 0) {
						int len = send(sockfd, header.c_str() + length - left, left, 0);
						if (len <= 0)
							break;
						left -= len;
					}
					if (left == 0) {
						char buf[1024] = { 0 };
						while (true) {
							int len = recv(sockfd, buf, sizeof(buf), 0);
							if (len <= 0)
								break;

						}
					}
				}
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

bool httpclient::get(std::string url, std::string file) {
	bool ret = false;
	pullitem *pi = new pullitem();
	pi->filepeer = url;
	pi->filelocal = file;
	ret = urlparse(url, pi->ui);
	if (ret) {
		std::lock_guard<std::recursive_mutex> gs(this->sockmt);
		_qpull.push(pi);
	}
	else {
		delete pi;
	}
	return ret;
}

NS_MONSTERLIVE_NET_END