#include "send_request.h"
#include "down_thread_arg.h"
#include "down_queue.h"
#include "clientConfig.h"
#include "get_proxy_list.h"


/**下载主体入口**/
void* start_down(void* tmp);


/**执行下载的线程函数**/
void * down_func(void *);


/**往下载队列里塞元素**/
int send_queue(clientConfig* conf, down_queue* q, int down_counter);

void get_proxy_host(clientConfig* conf, std::string& h);



