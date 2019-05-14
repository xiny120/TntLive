#pragma once
#include <mutex>
#include <thread>
#include <queue>
#include <map>
#include <list>

#define NS_MONSTERLIVE_NET_BEGIN namespace monsterlive { namespace net{
#define NS_MONSTERLIVE_NET_END }}


NS_MONSTERLIVE_NET_BEGIN

// http://www.pic98.com:8080/file/beautileg/samansha/201901011506.jpg?session=a231-dd33&token=aabbcc
class urlitem {
public:
	urlitem() {
		totallen = 0;
		locallen = 0;
		pulllen = 0;
		failtimes = 0;
		lastactive = 0;
		firsttry = 0;
	}
	std::string url;
	std::string fullpath; // http://www.pic98.com:8080/file/beautileg/samansha/201901011506.jpg#abc
	std::string query; // session=a231-dd33&token=aabbcc
	std::map<std::string, std::string> param; // {session =>a231-dd33,token=>aabbcc}
	std::string verb; // POST or GET
	std::string prot; // http
	std::string host; // www.pic98.com
	uint16_t port; // 8080
	std::string path; // /file/beautileg/samansha/201901011506.jpg
	std::string pathdir; // /file/beautileg/samasha
	std::string file; // 201901011506.jpg #�Ż�û��������������
	std::list<std::string> ip;	// dns result for www.pic98.com
	std::string officalhost;
	std::list<std::string> aliases;	// dns result for www.pic98.com
	int64_t contentlength;
	int64_t totallen;
	int64_t locallen;
	int64_t pulllen;
	int64_t failtimes;
	int64_t lastactive;
	int64_t firsttry;
	int64_t lastmodify;
	void active() {
		lastactive = time(nullptr);
	}


};

struct pullitem {
	std::string filepeer;
	std::string filelocal;
	std::string pathlocal;
	int32_t		encryption;
	urlitem ui;
};

typedef std::list<pullitem*> listpull;

class httpclient
{
private:
	httpclient();
	virtual ~httpclient();
public:
	static httpclient* me();
	virtual int Init();
	bool get(const std::string url, std::string& file,const std::string dir,int32_t);
	void pause() { mpause = true; };
	void start() { mpause = false; };
private:
	//���ݴ��������ڴ�
	virtual void run();
	bool mrun;
	bool mpause;
	//int sockfd;
	std::recursive_mutex sockmt;
	std::thread sockthr;
	listpull	_qpull;
	bool urlparse(std::string urlin,urlitem & out);
	std::string prepareheader(const urlitem*);
	bool isfileexists(const char*path);
	time_t str2time(const char* str);
	time_t getfiletime(const char* file);
	void setfiletime(const char* file, const char* time);


};
NS_MONSTERLIVE_NET_END
