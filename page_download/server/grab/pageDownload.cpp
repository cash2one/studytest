#include "grab/pageDownload.h"
#include "utils/Utils.h"
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>

CURLSH* pageDownload::share_handle = NULL;

/**
 * 将下载的网页数据写入文件的回调函数
 */
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream){
	size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
	return written;
}

/**
 * 开始下载网页，这是一个死循环，直到当前线程被杀时才结束
 */
int32_t pageDownload::run(){
	
	///开始死循环
	while(1){
		uint32_t counter = 0;
		while( cur_list.size() <= 0 ){
			counter++;
			fprintf(stderr, "pageDownload counter = %d\n", counter);
			get_page_list();
			if( counter > 3 && _queue->size() <= 0){
				return -2;
			}
		}
		while(cur_list.size() > 0){
			string line(cur_list.front());
			download_one_page(line);
			cur_list.pop_front();
		}
	}
	
	
	return 0;
}

//补充当前要下载的页面列表
int32_t pageDownload::get_page_list(){
	list<string> data;
	_queue->getQueue(10, data);
	if( data.size() > 0){
		cur_list.insert(cur_list.end(), data.begin(), data.end());
	}
	return 0;
}

///初始化全局的cURL句柄
int32_t pageDownload::init_curl_handler(){
	///连接句柄
	curl_handle = curl_easy_init();
	if(!curl_handle){
		return -1;
	}
	
	
	///头信息
	if( NULL == headers ){
		headers = curl_slist_append(headers, "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3");
		headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:30.0) Gecko/20100101 Firefox/30.0");
		headers = curl_slist_append(headers, "Connection: keep-alive");
		headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
		headers = curl_slist_append(headers, "Accept-Charset: utf-8,gb2312;q=0.7,*;q=0.7");
	}
	
	
	///设置一些公共属性
	curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "gzip,deflate");
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 3L);
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_FORBID_REUSE, 1);
	curl_easy_setopt(curl_handle, CURLOPT_HEADER,0L); 
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT , 3L);
	curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT  , 2L);
	
	//设置DNS缓存共享
	if (NULL == share_handle){
		share_handle = curl_share_init();  
		curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);  
	}  
	curl_easy_setopt(curl_handle, CURLOPT_SHARE, share_handle);  
	curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, 600);  
	
	return 0;
}

///下载一个页面
int32_t pageDownload::download_one_page(const string& line){
	if( init_curl_handler() != 0){
		return -2;
	}
    if( line.length() <= 0 || line.find('\t') == string::npos){
		return -1;
	}

	vector<string> tmp_line;
	Utils::explode(line, "\t", tmp_line);
	if (tmp_line.size() < 3){
		return -2;
	}
	
	string q = "__QUERY__";
    string file = tmp_line[2];
	Utils::trim(file,"\t\n ");
	string url = tmp_line[1];
	Utils::trim(url,"\t\n ");
	url.replace(url.find(q), q.length(), tmp_line[0]);
	
	
	///检测文件是否存在
	if( access(file.c_str(), R_OK) == 0){
		return -2;
	}
	
	///先下载到一个临时文件，等完全下载完了再把名称改过来
	string tmp_file = file;
	tmp_file.append(".tmp");
	if(NULL != fp){
		fclose(fp);
	}
	fp = fopen(tmp_file.c_str(), "w+");
	if(NULL == fp){
		return -3;
	}
	
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_FILE, fp);
	res = curl_easy_perform(curl_handle);
	fclose(fp);
	fp =  NULL;
	curl_easy_cleanup(curl_handle);
	
	///如果下载失败（是不是可以考虑再写回cur_list？）
    if(res != CURLE_OK){
		fprintf(stderr, "download url : %s failed, errmsg : %s\n", url.c_str(), curl_easy_strerror(res));
        unlink(tmp_file.c_str());
		cur_list.push_back(line);
		return -4;
	}

	
	///把名字改过来
	rename(tmp_file.c_str(), file.c_str());
	return 0;
}

///清理现场
pageDownload::~pageDownload(){
	curl_slist_free_all(headers);
	//curl_share_cleanup(share_handle );
}



