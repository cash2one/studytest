#include "parser_request/doParse.h"
#include "utils/Utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>



/**
 * 开始运行解析进程
 */
int32_t doParse::run(){
	vector<string> host_info;
	Utils::explode(host,":",host_info);
	if(host_info.size() != 2){
		fprintf(stderr,"[doParse::run] invalid host %s\n",host.c_str());
		return -3;
	}
    boost::shared_ptr<TTransport> socket(new TSocket(host_info[0].c_str(), atoi(host_info[1].c_str())));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    srpp_parserClient c(protocol);
	transport->open();
	client = &c;
	if( client->ping() != 0){
		fprintf(stderr,"[doParse::run] ping %s failed, exit......\n",host.c_str());
		return -3;
	}
	
	///开始处理解析请求
	while(1){
		//当前线程是否可以退出的判断
		while( cur_list.size() <= 0 ){
			get_page_list();
			if( _queue->size() <= 0){
				fprintf(stderr,"[doParse::run] cannot get more element from Queue,exit.. host[%s]\n", host.c_str());
				sleep(1);
			}
		}
		///开始解析操作
		while(cur_list.size() > 0){
			string line(cur_list.front());
			send_requst(line);
			cur_list.pop_front();
		}
	}
	
	transport->close();
	fprintf(stderr,"[doParse::run] thread finished and exit, host[%s]\n", host.c_str());
	return 0;
}

//开始发送请求
int32_t doParse::send_requst(const string& line){
	///切割原行
	vector<string> tmp_line;
	Utils::explode(line, "\t", tmp_line);
	
	//提取html_file及item_file
	string url = tmp_line[0];
	string html_file = tmp_line[1];
	string query = tmp_line[2];
	string item_file;
	conf->get_item_file(html_file, item_file);
	if( access(item_file.c_str(),R_OK) == 0){
		return -5;
	}
	
	//提取引擎
	string engine;
	conf->get_engine_id(html_file, engine);
	if( (conf->engine_info).find(engine) ==  (conf->engine_info).end() ){
		fprintf(stderr,"[doParse::send_requst] invalid engine %s\n",engine.c_str());
		return -6;
	}
	
	
	//发送解析请求
	parse_arg test;
	test.html_content = "";
	char extra[4096] = {0};
	sprintf(extra,"{ \
		\"html_dest\":\"root@%s:%s\", \
		\"url\":\"%s\",  \
		\"engine_id\":\"%s\", \
		\"task_id\":\"%d\", \
		\"ad_type\":\"0\", \
		\"query\":\"%s\"}",(conf->html_dest_host).c_str(),html_file.c_str(),url.c_str(),engine.c_str(),conf->task_id,query.c_str());
	test.extra = extra;
	test.engine = (conf->engine_info)[engine];
	test.ret_action = 5;
	string ret_str;
	
	//执行，并计算执行时间
	struct timeval tv_begin, tv_end;
	gettimeofday(&tv_begin, NULL);
	client->parseHtml(ret_str,test);
	gettimeofday(&tv_end, NULL);
	
	//校验是否下载完整
	if( ret_str.find("success") == string::npos ){
		fprintf(stderr,"operation failed,%s\n",ret_str.c_str());
		return -4;
	}
	
	FILE *fp = fopen(item_file.c_str(),"w");
	fwrite(ret_str.c_str(), ret_str.length(), 1, fp);
	fclose(fp);
	
	//打印结果
	long stime = tv_begin.tv_sec * 1000000 + tv_begin.tv_usec;
	long etime = tv_end.tv_sec * 1000000 + tv_end.tv_usec;
	long diff_t = etime - stime;
	float dt = (double)diff_t / 1000000.0;
	fprintf(stderr,"[doParse::send_requst] host[%s] time[%f]\n",host.c_str(), dt);

	return 0;
}

/**
 * 从队列中获取要解析的页面
 */
int32_t doParse::get_page_list(){
	list<string> data;
	_queue->getQueue(20, data);
	if( data.size() > 0){
		fprintf(stderr,"[doParse::get_page_list] get Queue size=%d\n",data.size());
		cur_list.insert(cur_list.end(), data.begin(), data.end());
	}
	return 0;
}


