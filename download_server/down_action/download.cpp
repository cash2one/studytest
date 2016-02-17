#include "download.h"
#include "utils/Utils.h"
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/stat.h>


using namespace std;

static char errorBuffer[CURL_ERROR_SIZE];


/**
 * 将下载的网页数据写入文件的回调函数
 */
static size_t write_data(char *ptr, size_t size, size_t nmemb, void *stream){
	FILE* f = (FILE*)stream;
	fwrite(ptr, size, nmemb, f);
	fflush(f);
	return size * nmemb;
}

download::download(downConfig* cnf, const std::string& proxy):curl_handle(NULL),haproxy(proxy),conf(cnf)  {
	if( init_something() != 0){
		fprintf(stderr,"download::download init failed\n");
	}
}


/** 遍历当前要下载的列表 **/
int32_t download::run(const vector<string>& down_list ){
	if( NULL == curl_handle){
		return -2;
	}

	//遍历当前要下载的列表
	vector<string>::const_iterator iter;
	for(iter=down_list.begin();iter!=down_list.end();iter++){
		download_one_page(*iter);
	}

	return 0;
}



/** 初始化 **/
int32_t download::init_something(){
	curl_handle = curl_easy_init();
	if(!curl_handle){
		fprintf(stderr,"[download::init_something] curl_easy_init failed\n");
		return -1;
	}
	
	//一些共享的东西
	curl_easy_setopt(curl_handle, CURLOPT_SHARE, conf->share_handle);  
	curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, 6000);  
	
	
	///设置一些公共属性
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "gzip,deflate");
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 3L);
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_HEADER,0L); 
	curl_easy_setopt(curl_handle, CURLOPT_HTTPGET,1); 
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, conf->headers);
	if( conf->timeout > 0){
		curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT , conf->timeout);
		curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT  , conf->timeout);
	}
	if(haproxy.length() > 10){
		fprintf(stderr,"[download::init_something] init curl_easy_handler USE Proxy %s\n",haproxy.c_str());
		curl_easy_setopt(curl_handle, CURLOPT_PROXY, haproxy.c_str()); 
	}
	
	return 0;
}


/** 开始下载 line：url   \t   localtion **/
int32_t download::download_one_page(const std::string& line){
   if( line.length() <= 0 || line.find('\t') == string::npos){
		return -1;
	}

	//切开
	vector<string> tmp_line;
	Utils::explode(line, "\t", tmp_line);
	if (tmp_line.size() < 2){
		return -2;
	}
	
	
	//url及location
	string url = tmp_line[0];
    string location = tmp_line[1];

	
	//本地的临时文件
	string tmp_file = conf->tmp_html_dir;
	Utils::rtrim(tmp_file,"/");
	string uuid_str;
	Utils::gen_uuid(uuid_str);
	tmp_file.append("/").append(uuid_str).append(".html");
	FILE *fp = fopen(tmp_file.c_str(), "w+");
	if(NULL == fp){
		perror("[download::download_one_page] write html file failed");
		return -3;
	}

	
	///执行下载操作
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)fp);
	CURLcode  res = curl_easy_perform(curl_handle);
	if(res != CURLE_OK){
		fclose(fp);
		unlink(tmp_file.c_str());
		fprintf(stderr, "[download::download_one_page] Failed[%s] errmsg[%s|%s]\n", 
				url.c_str(),errorBuffer,curl_easy_strerror(res));
		return -4;
	}
	
	
	///判断返回的状态码
	int http_status = 0;
	res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE , &http_status);
	if(http_status != 200){
		fclose(fp);
		unlink(tmp_file.c_str());
		fprintf(stderr, "[download::download_one_page] Failed[%s] errmsg[%s|%s] status[%d]\n", url.c_str(),errorBuffer,curl_easy_strerror(res),http_status);
		return -4;
	}
	
    fclose(fp);
	
	/**判断大小：1024字节以下的，绝B有问题**/
	struct stat buf; 
	stat(tmp_file.c_str(),&buf);
	if(buf.st_size <= 1024){
		unlink(tmp_file.c_str());
		return 0;
	}
	
	//调用shell文件处理
	string cmd = "/bin/bash ";
	cmd.append(conf->sendfile_shell).append(" ").append(tmp_file).append("  ").append(location).append(" >/dev/null 2>&1");
	FILE* fp1 = popen(cmd.c_str(),"r");
	fclose(fp1);

	return 0;
}

download::~download(){
	curl_easy_cleanup(curl_handle);
}

