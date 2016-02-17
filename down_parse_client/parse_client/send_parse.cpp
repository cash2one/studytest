#include "send_parse.h"
#include "utils/Utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "jsoncpp/jsoncpp_json.h"


using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;



/**开始不断的发起解析请求**/
int send_parse::run(){
    if( strlen(arg->parse_server) <= 0){
        return -4;
    }
	
	/** 提取parse server里的IP和port **/
	vector<string> host_info;
	Utils::explode(arg->parse_server,":",host_info);
	if(host_info.size() != 2){
		fprintf(stderr,"[send_parse::run] invalid host %s\n",arg->parse_server);
		return -3;
	}
	
	
	/**连接server端**/
    boost::shared_ptr<TTransport> socket(new TSocket(host_info[0].c_str(), atoi(host_info[1].c_str())));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));  
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    srpp_parserClient c(protocol);
	client = &c;
	try{
		transport->open();
		client->ping();
	}
	catch(exception &e){
		fprintf(stderr,"[send_parse::run] ping %s failed, errmsg[%s] exit......\n",  arg->parse_server, e.what());
		return -3;
	}
	fprintf(stderr,"[send_parse::run] ping %s successful\n",arg->parse_server);

	
	/**从队列里提取数据并发起解析请求**/
	list<string > cur_list;
	list<string >::iterator iter;
	do{
		get_page_list(cur_list);
		if(cur_list.size() <= 0 && arg->q->is_finish > 0){
			break;
		}
		
		//没取到数据
		if(cur_list.size() <= 0){
			sleep(2);
			continue;
		}
		
		//遍历取到的数据
		for(iter=cur_list.begin(); iter!=cur_list.end(); iter++){
			do_send_parse(*iter);
		}
		cur_list.clear();
	}while(1);
	
	transport->close();
	fprintf(stderr,"[send_parse::run] thread finished and exit, host[%s] task_id[%s]\n", \
				arg->parse_server, (arg->conf->task_id).c_str());
	return 0;
}


/**开始解析一个页面请求**/
int32_t send_parse::do_send_parse(const string& line){
	///切割原行
	vector<string> tmp_line;
	Utils::explode(line, "\t", tmp_line);
	
	
	//提取html_file及item_file
	string url = tmp_line[0];
	string html_file = tmp_line[1];
	if( access(html_file.c_str(),R_OK) != 0){
		return -1;
	}
	string query = "";
    for(vector<string>::size_type i=2; i<tmp_line.size(); i++){
        query.append("\t").append(tmp_line[i]);
    }
	Utils::trim(query,"\t\n ");

	
	
	
	string item_file="";
	string engine_id="";
	arg->conf->get_engine_item(html_file, engine_id, item_file);
	
	
	
	/**基础的参数校验**/
	if( access(item_file.c_str(),R_OK) == 0 || engine_id.length() <= 0){
		return -4;
	}
	
	
	
	/**读取html_file内容**/
	string html_content = "";
	FILE *fp = fopen(html_file.c_str(),"r");
	if(NULL == fp){
		perror("[send_parse::do_send_parse] open failed");
		return -4;
	}
	char tmp_str[40960] = {0};
	while( !feof(fp) ){
		fread(tmp_str,40960,1,fp);
		if( strlen(tmp_str) <= 0){
			continue;
		}
		html_content.append(tmp_str);
		bzero(tmp_str,40960);
	}
	fclose(fp);
	
	
	/**校验html页面大小**/
	if( html_content.length() < atoi((arg->conf->normal_page_limit_size).c_str())){
		fprintf(stderr,"[send_parse::do_send_parse] page size too small\n");
		unlink(html_file.c_str());
		return -5;
	}
	
	
	/**解析请求：将结果入库并返回item文件**/
	parse_arg test;
	Json::FastWriter writer;
	Json::Value arg_data;
	test.html_content = html_content;
	
	/**send_dest**/
	char sd[1024] = {0};
	sprintf(sd,"root@%s:%s",(arg->conf->dest_host).c_str(),item_file.c_str());
	arg_data["send_dest"] = sd;
	arg_data["engine_id"] = engine_id;
	arg_data["task_id"] = arg->conf->task_id;
	arg_data["ad_type"] = "0";
	arg_data["query"] = query;
	arg_data["html_dest"] = html_file;
	
	test.extra = writer.write(arg_data);
	test.engine = (arg->conf->engine_info)[engine_id];
	test.ret_action = 2;
	string ret_str;
	
	
	//执行，并计算执行时间
	struct timeval tv_begin, tv_end;
	gettimeofday(&tv_begin, NULL);
	client->parseHtml(ret_str,test);
	gettimeofday(&tv_end, NULL);

	
	//打印结果
	long stime = tv_begin.tv_sec * 1000000 + tv_begin.tv_usec;
	long etime = tv_end.tv_sec * 1000000 + tv_end.tv_usec;
	long diff_t = etime - stime;
	float dt = (double)diff_t / 1000000.0;
	fprintf(stderr,"[send_parse::do_send_parse] host[%s] time[%f] query[%s] errmsg[%s]\n", \
				arg->parse_server, dt,query.c_str(),ret_str.c_str());
	
	
	/**检查结果**/
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(ret_str.c_str(), root, false)){
		return -1;
	}
	if(!root.isMember("errno") || !root.isMember("errmsg")){
		return -1;
	}
	if(root["errno"].asInt() == 100){
		fprintf(stderr,"[send_parse::do_send_parse] delete %s\n", html_file.c_str());
		unlink(html_file.c_str());
	}

	return 0;

}


/**从队列里提取数据**/
int32_t send_parse::get_page_list(list<string >& data){
	data.clear();
	try{
		arg->q->getQueue(5, data);
	}
	catch(exception &e){
		fprintf(stderr,"[send_parse::get_page_list] exception[%s]\n",e.what());
	}
	return 0;
}




