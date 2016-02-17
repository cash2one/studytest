#ifndef _HOST2IP_H_
#define _HOST2IP_H_

#include<string>
#include<stdint.h>
#include<stdlib.h>

using namespace std;
class parseURL{
public:
	string ip;
	string uri;
	string host;
	int port;

	parseURL(const string& urlinfo):port(80),url(urlinfo){
	}
    parseURL():port(80){}
	
	int setURL(const string& urlinfo);
	int parse();
	~parseURL();
	
private:
	string url;
};

#endif
