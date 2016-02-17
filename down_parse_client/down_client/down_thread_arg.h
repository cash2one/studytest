#ifndef _DOWN_THREAD_ARG_H_
#define _DOWN_THREAD_ARG_H_

#include "down_queue.h"
#include "clientConfig.h"


class down_thread_arg{
public:
	down_thread_arg():q(NULL),conf(NULL){}


	/**队列指针**/
	down_queue* q;
	
	
	/**要请求的download server端地址**/
	char down_server[30];
	
	
	/**配置类指针**/
	clientConfig* conf;
};

#endif


