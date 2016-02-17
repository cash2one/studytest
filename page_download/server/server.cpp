// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "server/grab_page.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "utils/ConfigParser.h"
#include "utils/Global.h"
#include <limits.h>
#include <stdlib.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::page_download;

/**
 *此处仅将接收的参数存到文件里，并不做其他的检查。
 * 然后触发另外一个程序去处理相应的任务记录，
 * 此处理程序可随时中止并随时启动继续上次被中止的任务
 * 并由crontab监控处理程序的运行情况
 *
 */
class grab_pageHandler : virtual public grab_pageIf {
 public:
  grab_pageHandler() {
    // Your initialization goes here
  }

  int32_t ping() {
    // Your implementation goes here
    printf("ping\n");
	return 0;
  }

  int32_t start_grab(const crawler_arg& arg) {
    // Your implementation goes here
    string task_path;
    global_config.get_item("TASK_FILE_DIR",task_path);
    if( access(task_path.c_str(),W_OK) != 0){
        if( mkdir(task_path.c_str(),0755) != 0){
            return -1;
        }
    }
    char tmp[10] = {0};
    sprintf(tmp,"%d",arg.task_id);
    string task_file = task_path;
    task_file.append("/").append(tmp).append(".task");
    string task_tmp_file = task_file;
    task_tmp_file.append(".doing");
    ///这两个文件都不能存在
    if( access(task_file.c_str(),R_OK) == 0){
        return -4;
    }
    if( access(task_tmp_file.c_str(),R_OK) == 0){
        return -5;
    }

    //将任务信息写入文件
    int fd = open(task_file.c_str(), O_WRONLY|O_CREAT|O_EXCL, 0755);
    if(0 == fd){
        return -6;
    }
    string data = tmp;
    bzero(tmp,10);
    sprintf(tmp, "%d", arg.start);
    data.append("\t").append(tmp);
    bzero(tmp,10);
    sprintf(tmp, "%d", arg.limit);
    data.append("\t").append(tmp);
    string config_file;
    global_config.get_config_file(config_file);
    data.append("\t").append(config_file);
    ssize_t num = write(fd, data.c_str(), data.length());
	close(fd);
	if( num <= 0){
		return -7;
	}

	return 0;
  }

};

int main(int argc, char **argv) {
	if(argc < 2){
		fprintf(stderr, "invalid config file\n");
		exit(EXIT_FAILURE);
	}
	const char* conf_file = argv[1];
	if( access(conf_file,R_OK) != 0 ){
		fprintf(stderr, "read config file failed\n");
		exit(EXIT_FAILURE);
	}
	
	//转为realpath
	char *cpath = canonicalize_file_name(conf_file);
	if(cpath == NULL){
		perror("get config file realpath failed");
		exit(EXIT_FAILURE);
	}

	///解析配置文件
	global_config.set_config_file(cpath);

	///获了监听的端口
	string p;
	global_config.get_item("THRIFT_LISTEN_PORT",p);
	int port = atoi(p.c_str());

	shared_ptr<grab_pageHandler> handler(new grab_pageHandler());
	shared_ptr<TProcessor> processor(new grab_pageProcessor(handler));
	shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

	TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
	server.serve();
	free(cpath);
	return 0;
}

