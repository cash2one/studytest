#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<netdb.h>

#include "Http.h"

using namespace std;

Http::Http(){
	init_Http();
}

Http::Http(const string& req_url){
	set_url(req_url);
	init_Http();
}

Http::Http(const string& req_url, const map<string,string>& data){
	set_url(req_url);
	set_post_data(data);
	init_Http();
}

/**
  *请求URL直接替换 
  */
int Http::set_url(const string& req_url){
	if(req_url.length() > 0){
		url = req_url;
	}
	init_url();
	return 0;
}

/**
 *请求头信息，一项一项的覆盖
 */
int Http::set_request_header(const map<string,string>& h){
	if(h.size() <= 0){
		return -1;
	}
	
	//遍历之
	map<string,string>::const_iterator iter;
	for(iter=h.begin(); iter != h.end(); iter++){
		request_header[iter->first] = iter->second;
	}
	return 0;
}

/**
 *要发送的POST数据，也是一项一项的覆盖
 */
int Http::set_post_data(const map<string,string>& data){
	if(data.size() <= 0){
		return -1;
	}
	
	//遍历之
	map<string,string>::const_iterator iter;
	for(iter=data.begin(); iter != data.end(); iter++){
		post_data[iter->first] = iter->second;
	}
	return 0;
}

/**
 * 返回响应的状态码
 */
int Http::get_response_status(){
	return 0;
}

/**
 *返回响应的头信息
 */
int Http::get_response_header(map<string,string>& h){
	return 0;
}

/**
 *发起一个GET请求
 */
int Http::get(){
	string get_header = "GET ";
	get_header += urlInfo.uri;
	get_header += " HTTP/1.1\r\n";
	
	get_req_header();
	get_header = get_header + req_header_str;
    get_header += "\r\n";
fprintf(stderr,"get_header------------\n%s\n", get_header.c_str());
	return send_request(get_header);
}

/**
 *发起一个POST请求
 */
int Http::post(){
	//POST请求特有的头信息
	request_header["Content-Type"] = "application/x-www-form-urlencoded";
	string post_header = "POST ";
	post_header += urlInfo.uri;
	post_header += " HTTP/1.1\r\n";
	
	///拼装data信息
	string data_str;
	map<string,string>::const_iterator iter;
	for(iter=post_data.begin(); iter != post_data.end(); iter++){
		data_str += iter->first;
		data_str += "=";
		data_str += iter->second;
		data_str += "&";
	}
	
	///再次补充头信息
	char content_length[10] = {0};
	sprintf(content_length, "%lu", data_str.length());
	request_header["Content-Length"] = content_length;
	
	///正式的头信息
	get_req_header();
	string h = post_header + req_header_str;
	
	string post_str = h;
	post_str += "\r\n";
	post_str += data_str;
	
	///开始发送请求
	return send_request(post_str);
}

/**
 *初始化请求的头信息，拼装成字符串的形式
 */
int Http::get_req_header(){
	if(request_header.size() <= 0){
		return -1;
	}
	
	///开始拼装请求头信息
	map<string,string>::const_iterator iter;
	for(iter=request_header.begin(); iter != request_header.end(); iter++){
		req_header_str += iter->first;
		req_header_str += ": ";
		req_header_str += iter->second;
		req_header_str += "\r\n";
	}
	return 0;
}

/**
 *初始化socket句柄
 */
int Http::init_sock(){
    struct in_addr sip;
    inet_pton(AF_INET, urlInfo.ip.c_str(), &sip);
    struct sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(urlInfo.port);
    sin.sin_addr = sip;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr *sun = (struct sockaddr*)&sin;
    int ret = connect(sockfd, sun, sizeof(struct sockaddr));
    if(ret != 0){
        perror("connect");
        return -1;
    }
	return 0;
}

/**
 *发起一个请求信息
 */
int Http::send_request(const string& data){
    init_sock();
	if(data.length() <= 0 || sockfd <= 0){
		return -1;
	}
	
	///发送数据
	int ret = send(sockfd, data.c_str(), data.length(), 0);
	if(ret <= 0){
		perror("send net data");
		return -1;
	}
	
	///接收数据
	get_response();
	return 0;
}

/**
 *读取HTTP响应头信息
 */
int Http::get_response(){
	///先读取头信息
	char header[20480] = {0};
	int ret = recv(sockfd,header,20480,MSG_PEEK);
	if(ret <= 0){
		perror("recv header data failed");
		return -1;
	}
	
	///寻找第一个\r\n\r\n
	char* end_pos = strstr(header,"\r\n\r\n");
	if( NULL == end_pos){
		return -2;
	}
	int header_len = end_pos - header;
    bzero(end_pos,20480-(end_pos-header));
	
	///解析头信息
	char* buf=NULL;
	int tmp_pos = 0;
	buf = strtok(header,"\r\n");
	while( buf != NULL){
		string tmp(buf);
		buf = strtok(NULL,"\r\n");
		tmp_pos = tmp.find(":");
        if(tmp_pos <= 0){
			tmp_pos = tmp.find("HTTP/1.1");
			if(tmp_pos >= 0){
				tmp_pos = tmp.find_first_of(" ");
				tmp = tmp.substr( tmp_pos + 1,tmp.length() - tmp_pos - 1);
				tmp = tmp.substr(0,tmp.find_first_of(" "));
				response_status = atoi(tmp.c_str());
				fprintf(stderr,"response status: %d\n",response_status);
				continue;
			}
			fprintf(stderr,"invalid header:%s\n",buf);
            continue;
        }
		reponse_header[tmp.substr(0,tmp_pos)] = tmp.substr(tmp_pos+1,tmp.length()-tmp_pos-1);
	}
	
	bzero(header,20480);;
	recv(sockfd,header,header_len+4,0);
	
	///开始读取body信息
    map<string,string>::iterator iter;
	iter = reponse_header.find("Content-Length");
	if( iter != reponse_header.end() ){
		int body_len = atoi(reponse_header["Content-Length"].c_str());
		char* body = (char*)malloc(sizeof(char)* body_len + 1);
        *body = '\0';
		recv(sockfd,body,body_len,0);
		fprintf(stderr,"%s\n",body);
        free(body);
		return 0;
	}
	
	iter = reponse_header.find("Transfer-Encoding");
	if(iter == reponse_header.end()){
		fprintf(stderr,"read http body failed\n");
		return -3;
	}
	string chunked = reponse_header["Transfer-Encoding"];
	tmp_pos = chunked.find_first_not_of(" ");
	chunked = chunked.substr(tmp_pos,chunked.length()-tmp_pos);
	if(chunked.compare("chunked") != 0){
		fprintf(stderr,"read http body failed\n");
		return -4;
	}
	
    long chunked_len = 0;
	do{
        bzero(header,20480);
		recv(sockfd,header,10,MSG_PEEK);
		buf = strtok(header,"\r\n");
fprintf(stderr,"\nbuf:%s\n",buf);
		tmp_pos = strlen(buf);
		recv(sockfd,header,tmp_pos+2,0);
		chunked_len = strtol(buf,NULL,16);
		fprintf(stderr,"\n----------|%ld|\t|%s|-------------\n",chunked_len,buf);
		while(chunked_len > 20480){
            bzero(header,20480);
			recv(sockfd,header,20480,0);
fprintf(stderr,"%s\n",header);
			chunked_len = chunked_len - 20480;
		}
        bzero(header,20480);
		recv(sockfd,header,chunked_len,0);
fprintf(stderr,"%s\n",header);
	}while(chunked_len > 0);
	
	return 0;
}

/**
 *初始化一些参数，如头信息
 */ 
int Http::init_Http(){
	request_header["Accept-Language"] = "zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3";
	request_header["User-Agent"] = "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:30.0) Gecko/20100101 Firefox/30.0";
    request_header["Connection"] = "keep-alive";
	request_header["Accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
	request_header["Accept-Charset"] = "utf-8,gb2312;q=0.7,*;q=0.7 ";
	request_header["Cache-Control"] = "max-age=0";
	
	return 0;
}

int Http::init_url(){
	if(url.length() <= 0){
		return -1;
	}
	urlInfo.setURL(url);
	urlInfo.parse();
    string host_info = urlInfo.host;
    host_info += ":";
    char port_str[5] = {0};
    sprintf(port_str,"%d",urlInfo.port);
    request_header["Host"] = host_info + port_str;
fprintf(stderr,"url=%s\turi=%s\tIP=%s\tport=%d\n", url.c_str(),urlInfo.uri.c_str(),urlInfo.ip.c_str(),urlInfo.port);
	return 0;
}
/**
 * 开始析构了
 */
Http::~Http(){

}
