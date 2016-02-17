#ifndef _HTTP_H_
#define _HTTP_H_

#include<map>
#include<string>
#include<string.h>
#include "parseURL.h"

class Http{
public:
	Http();
	Http(const std::string& req_url);
	Http(const std::string& req_url, const std::map<std::string,std::string>& data);
	~Http();
	
	int set_url(const std::string& req_url);
	int set_request_header(const std::map<std::string,std::string>& h);
	int set_post_data(const std::map<std::string,std::string>& data);
	int get_response_status();
	int get_response_header(std::map<std::string,std::string>& h);
	
	int get();
	int post();
	
private:
	std::string url;
	std::map<std::string,std::string> post_data;
	std::map<std::string,std::string> request_header;
	std::map<std::string,std::string> reponse_header;
	int sockfd;
	int response_status;
	std::string req_header_str;
	parseURL urlInfo;
	
	int init_Http();
	int init_url();
	int get_req_header();
	int init_sock();
	int send_request(const std::string& data);
	int get_response();
};
#endif
