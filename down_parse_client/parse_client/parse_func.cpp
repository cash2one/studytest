#include "parse_func.h"
#include <memory>
#include "utils/Thread.h"
#include "utils/Utils.h"
#include "utils/MD5.h"
#include "mysql/mark_oss.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>


using namespace std;




/**主体入口**/
void* start_parse(void* tmp){
	clientConfig* conf = (clientConfig*)tmp;
	if( access((conf->seed_file).c_str(),R_OK) != 0 || conf->page_counter <= 0){
		fprintf(stderr,"[start_parse] read seed_file exists,%s\n",(conf->seed_file).c_str());
		return (void*)NULL;
	}
	
	/**初始化队列**/
	parse_queue _queue;
	check_sogou_parser(conf);

	
	/**启动线程，有多少服务端就启动多少线程**/
	int thread_num = (conf->parse_hosts).size();
    parse_thread_arg** args = (parse_thread_arg**)malloc(sizeof(parse_thread_arg*)*thread_num);
    bzero(args,sizeof(parse_thread_arg*)*thread_num);
	for(int i = 0; i < thread_num; i++){
        args[i] = new parse_thread_arg();
		bzero(args[i]->parse_server,30);
		args[i]->q = &_queue;
		strcpy(args[i]->parse_server,((conf->parse_hosts)[i%thread_num]).c_str());
		args[i]->conf = conf;
	}
	Thread obj(thread_num);
	obj.createMutilThread(parse_func, (void**)args);
	
	
	/**分配**/
	int cnt;
	do{
		cnt = send_queue(conf, &_queue);
		while( _queue.queue_size() > 0){
			sleep(3);
		}
		sleep(3);
		conf->import_db();
	}while( cnt > 0 );
	
	
	/**让子进程退出，并设置自己退出的标志，也好让下载线程一起退出**/
	_queue.is_finish = 1;
	conf->parse_thread_exit = 1;
	obj.joinThread();
	
	
	
	/**释放内存**/
	fprintf(stderr,"[start_parse] free malloc space........\n");
	for(int i = 0; i < thread_num; i++){
        delete args[i];
	}
	free(args);
	
	fprintf(stderr,"[start_parse] parse thread start exit...............\n");
	return (void*)NULL;
}



/**执行解析的线程函数**/
void* parse_func(void *tmp){
	parse_thread_arg* larg = (parse_thread_arg*)tmp;
	send_parse obj(larg);
	obj.run();
	return (void*)NULL;
}


/**往解析队列里塞元素**/
int send_queue(clientConfig* conf, parse_queue* q){
	/**遍历种子文件**/
	FILE *fp = fopen((conf->seed_file).c_str(),"r");
	if(NULL == fp){
		perror("open seed file failed");
		return 0;
	}
	
	char tmp[4096] = {0};
	list<string> cur_list;
	int counter = 0;
	while( !feof(fp) ){
		fgets(tmp,4096,fp);
		if( strlen(tmp) <= 0){
			continue;
		}
		string line(tmp);
		Utils::trim(line,"\t\n\r ");
		bzero(tmp,4096);
		if(line.find('\t') == string::npos){
			continue;
		}
		
		//切割，检查
		vector<string> items;
		Utils::explode(line,"\t",items);
		if( items.size() < 3){
			continue;
		}
		string url = items[0];
		string local_file = items[1];
		string query = items[2];
		string item_file,engine_id;
		conf->get_engine_item(local_file, engine_id, item_file);
		
		/**检查文件是否存在及其大小**/
		if( access(item_file.c_str(),R_OK) == 0 ){
			continue;
		}
		
		//放到待解析队列里
		cur_list.push_back(line);
		if( (counter++) % 20 == 0){
			try{
				q->addQueue(cur_list);
				cur_list.clear();
			}
			catch(exception &e){
				fprintf(stderr,"[parse_func::send_queue] exception[%s]\n",e.what());
			}
		}
	}
	
	//剩下的全部放进去
	if(cur_list.size() > 0){
		try{
			q->addQueue(cur_list);
			cur_list.clear();
		}
		catch(exception &e){
			fprintf(stderr,"[parse_func::send_queue] exception[%s]\n",e.what());
		}
	}
	fclose(fp);
	return counter;
}


/**如果只有sogou的，则启用sogou专属解析机器**/
void check_sogou_parser(clientConfig* conf){
	if( (conf->sogou_parse_hosts).size() <= 0){
		return;
	}
	int is_all_sogou = 0;
	map<string,string>::iterator iter;
	for(iter=(conf->engine_info).begin(); iter!=(conf->engine_info).end(); iter++){
		if( (iter->second).find("sogou") == string::npos ){
			is_all_sogou ++;
		}
	}
	if(is_all_sogou > 0){
		return;
	}
	
	/**合并解析机器**/
	vector<string>:: iterator iter_vec;
	for(iter_vec=(conf->sogou_parse_hosts).begin(); iter_vec!=(conf->sogou_parse_hosts).end(); iter_vec++){
		(conf->parse_hosts).push_back(*iter_vec);
	}

}
