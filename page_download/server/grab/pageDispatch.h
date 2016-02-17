#ifndef _PAGEDISPATCH_H_
#define _PAGEDISPATCH_H_

#include "threadManage/Queue.h"

/**
 * 读取种子文件，并将种子分发到各下载线程里去
 */
class pageDispatch{

public:
	pageDispatch(){}
	pageDispatch(Queue* q, string file) : _queue(q), seed_file(file) {}
	
	int32_t run();
	
private:
	Queue* _queue;
	string seed_file;
	
	list<string> cur_list; //已读取到的list
};

#endif

