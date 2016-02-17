#include "send_request.h"
#include "utils/Utils.h"



using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;


int send_request::run(){
    if( strlen(arg->down_server) < 9 ){
        fprintf(stderr,"[send_request::run] error\n");
        return -2; 
    }   
	
	/** 提取download server里的IP和port **/
	vector<string> host_info;
	Utils::explode(arg->down_server,":",host_info);
	if(host_info.size() != 2){
		fprintf(stderr,"[send_request::run] invalid host %s\n",arg->down_server);
		return -3;
	}
	
	
	/**连接server端**/
    boost::shared_ptr<TTransport> socket(new TSocket(host_info[0].c_str(), atoi(host_info[1].c_str())));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));  
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    downServiceClient client(protocol);
	try{
		transport->open();
		client.ping();
	}
	catch(exception &e){
		fprintf(stderr,"[send_request::run] ping %s failed, errmsg[%s] exit......\n",  arg->down_server, e.what());
		return -3;
	}
	fprintf(stderr,"[send_request::run] ping %s successful\n",  arg->down_server);
	
	
	/**容器**/
	map<string, list<string> > data;
	map<string, list<string> >::iterator iter;
	list<string>::iterator iter_list;
	do{
		get_page_list(data);
		if(data.size() <= 0 && arg->q->is_finish > 0){
			break;
		}
		
		//没取到数据
		if(data.size() <= 0){
			sleep(2);
			continue;
		}
		
		//遍历取到的数据
		for(iter=data.begin(); iter!=data.end(); iter++){
			vector<string> cur_list;
			string proxy_host = iter->first;
			for(iter_list=(iter->second).begin(); iter_list!=(iter->second).end(); iter_list++){
				cur_list.push_back(*iter_list);
			}
			if(cur_list.size() <= 0){
				continue;
			}
			//开始发起请求
			double dt = client.run_down(cur_list, proxy_host);
			fprintf(stderr,"[send_request::run] download %d pages task_id[%s] time[%f] host[%s] proxy[%s]\n", \
					cur_list.size(),(arg->conf->task_id).c_str(),dt,arg->down_server, proxy_host.c_str());
		}

	}while(1);
	
	transport->close();
	fprintf(stderr,"[send_request::run] thread finished and exit, host[%s] task_id[%s]\n", \
					arg->down_server,(arg->conf->task_id).c_str());
	return 0;
}


/**从队列中获取一些**/
int32_t send_request::get_page_list(map<string, list<string> >& data){
	data.clear();
	arg->q->getQueue(data);
	return 0;
}


