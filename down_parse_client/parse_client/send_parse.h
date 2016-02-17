#ifndef _SEND_PARSE_H_
#define _SEND_PARSE_H_
#include <sys/time.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include "parse_client_thrift/srpp_parser.h"
#include "clientConfig.h"
#include "parse_thread_arg.h"
#include <string>
#include <vector>
#include <stdint.h>


class send_parse{
public:
	send_parse(parse_thread_arg* a):arg(a){}

	int run();



private:
	/**参数**/
	parse_thread_arg* arg;
	
	
	/**客户端句柄**/
	srpp_parserClient* client;
	
	
	/**从队列中获取要下载的URL列表**/
	int32_t get_page_list(std::list<std::string >& data); 
	
	
	/**解析一个页面**/
	int32_t do_send_parse(const std::string& line);
};




#endif 



