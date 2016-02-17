#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TTransportUtils.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/file.h>
#include<fcntl.h>
#include "downService.h"
#include "download.h"

using boost::shared_ptr;
using namespace std;

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;



/** thrift钦定的类 **/
class downServiceHandler : virtual public downServiceIf {
public:
	downServiceHandler(const string& config) {
		conf_file = config;
	}

	int32_t ping() {
		return 0;
	}

	double run_down(const std::vector<std::string> & arg, const std::string& proxy_host) {
        fprintf(stderr,"[downServiceHandler] receive proxy %s\n",proxy_host.c_str());
        vector<string>::const_iterator iter;
        for(iter=arg.begin(); iter!=arg.end(); iter++){
            fprintf(stderr,"arg %s\n",(*iter).c_str());
        }
		struct timeval tv_begin, tv_end;
		gettimeofday(&tv_begin, NULL);
		
		//开始下载
		downConfig conf(conf_file);
		download obj(&conf, proxy_host);
		obj.run(arg);
		
		gettimeofday(&tv_end, NULL);
		long stime = tv_begin.tv_sec * 1000000 + tv_begin.tv_usec;
		long etime = tv_end.tv_sec * 1000000 + tv_end.tv_usec;
		long diff_t = etime - stime;
		double dt = (double)diff_t / 1000000.0;
		return dt;
	}
	
	
private:
	string conf_file;
};


/**  入口方法  **/
int main(int argc, char **argv) {
	if(argc < 3){
		fprintf(stderr, "USAGE\n");
		fprintf(stderr,"\t%s\t<port>\t<config_file>\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	int port = atoi(argv[1]);
	const string global_config_file = argv[2];
	
	
	//判断配置文件是否可读
	if( access(global_config_file.c_str(),R_OK) != 0 ){
		fprintf(stderr, "read config file failed\n");
		exit(EXIT_FAILURE);
	}
    int fd = open(global_config_file.c_str(), O_WRONLY);
	if( fd <= 0 ){
		perror("open config file failed\n");
		exit(EXIT_FAILURE);
	}
	if( flock(fd, LOCK_EX|LOCK_NB) < 0){
		fprintf(stderr, "lock config file failed:%s\n",global_config_file.c_str());
		exit(EXIT_FAILURE);
	}

	
	shared_ptr<downServiceHandler> handler(new downServiceHandler(global_config_file));
	shared_ptr<TProcessor> processor(new downServiceProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
    TThreadedServer server(processor,serverTransport,transportFactory,protocolFactory);
	
	
	fprintf(stderr, "start downService server...\n");  
	server.serve();  
	
	flock(fd, LOCK_UN);
	close(fd);
	return 0;
}


