#ifndef _PARSERDISPATCH_H_
#define _PARSERDISPATCH_H_
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/file.h>
#include "threadManage/Queue.h"
#include "parser_request/parserConfig.h"
#include <set>

using namespace std;

/**
 * 读取种子文件，并将种子分发到各下载线程里去
 */
class parserDispatch{

public:
	parserDispatch(){}
	parserDispatch(Queue* q, const string& file, parserConfig* c) : _queue(q), seed_file(file),conf(c) {}
	
	int32_t run();
	
private:
	Queue* _queue;
	string seed_file;
	
	list<string> cur_list; //已读取到的list
	
	//尚未完成的文件列表
	list<string> seed_list;
	
	//配置文件
	parserConfig* conf;
	
	//填充seed_map
	int32_t insert_seed_list();
	
	//检查下载进程是否已结束
	int32_t check_down_finish();
};


#endif

