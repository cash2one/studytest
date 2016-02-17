#include "down/pageDownload.h"
#include "utils/Utils.h"
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

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

/**
 * 开始下载网页，这是一个死循环，直到当前线程被杀时才结束
 */
int32_t pageDownload::run(){
	init_curl_header();
	
	///开始死循环
	while(1){
		uint32_t counter = 0;
		while( cur_list.size() <= 0 ){
			counter++;
			fprintf(stderr, "[pageDownload::run] current list is empty, counter = %d\n", counter);
			get_page_list();
			if( counter > 3 && _queue->size() <= 0){
				curl_slist_free_all(headers);
				curl_share_cleanup(share_handle );
				fprintf(stderr,"[pageDownload::run] download thread exit: %lu\n", pthread_self());
				return -2;
			}
		}
		while(cur_list.size() > 0){
			string line(cur_list.front());
			download_one_page(line);
			cur_list.pop_front();
		}
	}
	
	curl_slist_free_all(headers);
	curl_share_cleanup(share_handle );
	fprintf(stderr,"[pageDownload::run] download thread exit: %lu\n", pthread_self());
	
	
	return 0;
}

//补充当前要下载的页面列表
int32_t pageDownload::get_page_list(){
	list<string> data;
	_queue->getQueue(10, data);
	if( data.size() > 0){
		fprintf(stderr,"[pageDownload::get_page_list] get Queue size=%d\n",data.size());
		cur_list.insert(cur_list.end(), data.begin(), data.end());
	}
	return 0;
}

///下载一个页面，line的格式：url  \t  local_file
int32_t pageDownload::download_one_page(const string& line){
    if( line.length() <= 0 || line.find('\t') == string::npos){
		return -1;
	}

	vector<string> tmp_line;
	Utils::explode(line, "\t", tmp_line);
	if (tmp_line.size() < 2){
		return -2;
	}
	
	//url及要下载的页面
	string url = tmp_line[0];
    string file = tmp_line[1];
	
	///检测文件是否存在
	if( access(file.c_str(), R_OK) == 0){
		return -2;
	}
	
	///先下载到一个临时文件，等完全下载完了再把名称改过来
	string tmp_file = file;
	tmp_file.append(".tmp");

	FILE *fp = fopen(tmp_file.c_str(), "w+");
	if(NULL == fp){
		perror("[pageDownload::download_one_page] write html file failed");
		return -3;
	}

	//连接句柄
	CURL* curl_handle = curl_easy_init();
	if(!curl_handle){
		fclose(fp);
		unlink(tmp_file.c_str());
		fprintf(stderr,"[pageDownload::download_one_page] curl_easy_init failed\n");
		return -1;
	}
	init_curl_handler(curl_handle);
	
	///执行下载操作
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)fp);
	CURLcode  res = curl_easy_perform(curl_handle);
	if(res != CURLE_OK){
		fclose(fp);
		unlink(tmp_file.c_str());
		curl_easy_cleanup(curl_handle);
		fprintf(stderr, "[pageDownload::download_one_page] Failed[%s] errmsg[%s|%s]\n", 
				url.c_str(),errorBuffer,curl_easy_strerror(res));
		return -4;
	}
	
	///判断是否下载成功
	double length = 0;
	res = curl_easy_getinfo(curl_handle, CURLINFO_SIZE_DOWNLOAD , &length);
	int http_status = 0;
	res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE , &http_status);
	if(res != CURLE_OK || http_status != 200){
		fclose(fp);
		unlink(tmp_file.c_str());
		curl_easy_cleanup(curl_handle);
		fprintf(stderr, "[pageDownload::download_one_page] Failed[%s] errmsg[%s|%s] status[%d] downSize[%d]\n", url.c_str(),errorBuffer,curl_easy_strerror(res),http_status,(int)length);
		return -4;
	}
	
    fclose(fp);
	curl_easy_cleanup(curl_handle);

	///把名字改过来
	rename(tmp_file.c_str(), file.c_str());
	return 0;
}

int32_t pageDownload::init_curl_handler(CURL* curl_handle){
	if(curl_handle == NULL){
		return -1;
	}
	
	//设置DNS缓存共享
	if(share_handle == NULL){
		share_handle = curl_share_init();
		curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);  
	}
	curl_easy_setopt(curl_handle, CURLOPT_SHARE, share_handle);  
	curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, 6000);  
	
	///设置一些公共属性
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "gzip,deflate");
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 3L);
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_FORBID_REUSE, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_MAXCONNECTS, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_HEADER,0L); 
	curl_easy_setopt(curl_handle, CURLOPT_HTTPGET,1); 
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT , 30L);
	curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT  , 30L);
	if(haproxy.length() > 10){
		fprintf(stderr,"[pageDownload::init_curl_handler] init curl_easy_handler USE Proxy %s\n",haproxy.c_str());
		curl_easy_setopt(curl_handle, CURLOPT_PROXY, haproxy.c_str()); 
	}
	
	return 0;
}

///初始化header
int32_t pageDownload::init_curl_header(){
	if( NULL != headers){
		return -1;
	}
	///头信息
	headers = curl_slist_append(headers, "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3");
	headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:30.0) Gecko/20100101 Firefox/30.0");
	headers = curl_slist_append(headers, "Connection: keep-alive");
	headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	headers = curl_slist_append(headers, "Accept-Charset: utf-8;q=0.7,*;q=0.7");
	return 0;
}

///清理现场
pageDownload::~pageDownload(){
	
}



