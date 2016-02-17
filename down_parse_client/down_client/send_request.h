#ifndef _SEND_REQUEST_H_
#define _SEND_REQUEST_H_
#include <sys/time.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include "down_client_thrift/downService.h"
#include "clientConfig.h"
#include "down_thread_arg.h"
#include <string>
#include <vector>

class send_request{

public:
	send_request(down_thread_arg* a):arg(a){}

	int run();

private:
	
	/**参数**/
	down_thread_arg* arg;
	
	
	/**从队列中获取要下载的URL列表**/
	int32_t get_page_list(std::map<std::string, std::list<std::string> >& data); 
};


#endif


