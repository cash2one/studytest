#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<netdb.h>

#include "parseURL.h"
#include <boost/regex.hpp>

using namespace boost;
using namespace __gnu_cxx;

int parseURL::parse(){
	if(url.length() <= 5){
		return -1;
	}

	///是否以http://开头，是的话就去掉
	string http("http://");
	if(http.compare(url.substr(0,7)) == 0 ){
		url = url.substr(7,url.length()-7);
	}
	
	///截取host
	if(url.find_first_of("/",0) != string::npos){
		int pos = url.find_first_of("/",0);
		host = url.substr(0,pos);
		uri = url.substr(url.find_first_of("/",0),url.length()-pos);
	}
	else{
		host = url;
		uri = "/";
	}
	
	//若host里含有端口号
    port = 80;
	if(host.find_first_of(":",0) != string::npos){
		int pos = host.find_first_of(":",0);
		port = atoi(host.substr(pos+1,host.length()-pos-1).c_str());
		host = host.substr(0,pos);
	}
	
	///host是IP还是域名
	regex pattern("[\\d]+\\.[\\d]+\\.[\\d]+\\.[\\d]+");
	if(regex_match(host,pattern)){
		ip = host;
	}
	else{
		struct hostent *ret;
		ret = gethostbyname(host.c_str());
		if(ret->h_addr_list[0] != NULL){
			ip = inet_ntoa(*((struct in_addr *)ret->h_addr_list[0]));
		}
		else{
			return -2;
		}
	}
	
	return 0;
}

int parseURL::setURL(const string& urlinfo){
	url = urlinfo;
	return 0;
}

parseURL::~parseURL(){

}
