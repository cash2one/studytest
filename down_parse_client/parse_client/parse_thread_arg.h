#ifndef _PARSE_THREAD_ARG_H_
#define _PARSE_THREAD_ARG_H_


#include "parse_queue.h"
#include "clientConfig.h"


class parse_thread_arg{
public:
	parse_thread_arg():q(NULL),conf(NULL){}


	/**队列指针**/
	parse_queue* q;
	
	
	/**要请求的parse server端地址**/
	char parse_server[30];
	
	
	/**配置类指针**/
	clientConfig* conf;
};


#endif


