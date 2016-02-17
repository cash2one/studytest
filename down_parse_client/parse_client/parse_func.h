#include "parse_queue.h"
#include "send_parse.h"
#include "clientConfig.h"
#include "parse_thread_arg.h"
#include <set>



/**主体入口**/
void* start_parse(void* tmp);


/**执行的线程函数**/
void* parse_func(void *);


/**往队列里塞元素**/
int send_queue(clientConfig* conf, parse_queue* q);


/**单独检查所解析的是不是只有sogou引擎**/
void check_sogou_parser(clientConfig* conf);




