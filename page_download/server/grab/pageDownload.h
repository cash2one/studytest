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
	pageDownload(Queue* q) : _queue(q), curl_handle(NULL), headers(NULL), fp(NULL)  {}
	int32_t run(); //开始下载，这是一个死循环

private:
	//当前需要下载的页面list
	list<string> cur_list; 
	
	FILE* fp;
	
	//CURL句柄
	CURL* curl_handle;
	CURLcode res;
	curl_slist *headers;
	static CURLSH* share_handle;
	
	//队列句柄
	Queue* _queue;
	
	//补充当前要下载的页面列表
	int32_t get_page_list(); 
	
	///初始化全局的cURL句柄
	int32_t init_curl_handler();
	
	///下载一个页面
	int32_t download_one_page(const string& line);
};


#endif

