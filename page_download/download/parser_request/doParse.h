#ifndef _DOPARSE_H_
#define _DOPARSE_H_
#include "parser_request/parserConfig.h"
#include <sys/time.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include "parser_thrift/srpp_parser.h"
#include "threadManage/Queue.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

class doParse{
public:
	doParse(const string& h, Queue* q, parserConfig* c):
						_queue(q),
						host(h),
						conf(c),
						client(NULL){
	}
	int32_t run();

private:
	//要连接的服务端地址：ip:port格式
	string host;
	
	//当前需要解析的页面的路径列表
	list<string> cur_list; 
	
	//thrift客户端
	srpp_parserClient* client;
	
	//队列句柄
	Queue* _queue;
	
	//配置操作类
	parserConfig *conf;
	
	//补充当前要解析的页面列表
	int32_t get_page_list(); 
	
	//读取一个页面的内容并发送解析请求
	int32_t send_requst(const string& html_file);
};
#endif

