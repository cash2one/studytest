#ifndef _PAGEDOWNLOAD_H_
#define _PAGEDOWNLOAD_H_
#include<curl/curl.h>
#include "threadManage/Queue.h"

using namespace std;

/**
 * 网页下载控制类
 */
class pageDownload{

public:
	~pageDownload();
	pageDownload(Queue* q,const string& proxy) : _queue(q),headers(NULL),share_handle(NULL),haproxy(proxy)  {}
	int32_t run();

private:
	//当前需要下载的页面list
	list<string> cur_list; 
	
	int page_size;
	
	//代理
	string haproxy;
	
	//CURL句柄
	curl_slist *headers;
	CURLSH* share_handle;
	
	//队列句柄
	Queue* _queue;
	
	//补充当前要下载的页面列表
	int32_t get_page_list(); 
	
	///获取cURL句柄
	int32_t init_curl_handler(CURL*);
	
	//初始化header
	int32_t init_curl_header();
	
	///下载一个页面
	int32_t download_one_page(const string& line);
};


#endif

