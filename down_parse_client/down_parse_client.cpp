#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/file.h>
#include<fcntl.h>
#include "task_server_thrift/down_parse_client_serviceHandler.h"


using namespace std;
using boost::shared_ptr;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;



/**参数是配置文件、port**/
int main(int argc, char* argv[]){
	if(argc < 3){
		fprintf(stderr,"invalid arg 1");
		exit(1);
	}
	const char* conf_file = argv[1];
    int port = atoi(argv[2]);
	
	//判断配置文件是否可读
	if( access(conf_file,R_OK) != 0 ){
		fprintf(stderr, "read config file failed\n");
		exit(EXIT_FAILURE);
	}
    int fd = open(conf_file, O_WRONLY);
	if( fd <= 0 ){
		perror("open config file failed\n");
		exit(EXIT_FAILURE);
	}
	if( flock(fd, LOCK_EX|LOCK_NB) < 0){
		fprintf(stderr, "lock config file failed:%s\n",conf_file);
		exit(EXIT_FAILURE);
	}
	

	/**启动服务端**/
	shared_ptr<down_parse_client_serviceHandler> handler(new down_parse_client_serviceHandler(conf_file));
	shared_ptr<TProcessor> processor(new down_parse_client_serviceProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	
	
    //TThreadedServer server(processor,serverTransport,transportFactory,protocolFactory);

	
    shared_ptr<ThreadManager> thread_manager = ThreadManager::newSimpleThreadManager();
    shared_ptr<PosixThreadFactory> thread_factory(new PosixThreadFactory());
    thread_manager->threadFactory(thread_factory);
    thread_manager->start();
    TNonblockingServer server(processor, protocolFactory, port, thread_manager);
	
	
	fprintf(stderr, "start server...\n");  
	server.serve();  
	flock(fd, LOCK_UN);
	close(fd);
	return 0;
}



