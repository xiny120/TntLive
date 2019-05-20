#pragma once
#include <string>
#include <list>
#include <map>
class webapi
{
	class urlitem {
	public:
		urlitem() {
		}
		//std::string url;
		std::string fullpath; // http://www.pic98.com:8080/file/beautileg/samansha/201901011506.jpg#abc
		std::string query; // session=a231-dd33&token=aabbcc
		std::map<std::string, std::string> param; // {session =>a231-dd33,token=>aabbcc}
		std::string verb; // POST or GET
		std::string prot; // http
		std::string host; // www.pic98.com
		uint16_t port; // 8080
		std::string path; // /file/beautileg/samansha/201901011506.jpg
		std::string pathdir; // /file/beautileg/samasha
		std::string file; // 201901011506.jpg 
		std::list<std::string> ip;	// dns result for www.pic98.com
		std::string officalhost;
		std::list<std::string> aliases;	// dns result for www.pic98.com
	};
private:
	webapi();
	~webapi();
public:
	static webapi* me();
	std::string post(char* url,char* sid, char* par);
	bool urlparse(char* urlin,urlitem & out);
	std::string prepareheader(const urlitem* ui,char*,std::string post);
};

