#include "down_func.h"
#include<memory>
#include "utils/Thread.h"
#include "utils/Utils.h"
#include "mysql/DB.h"
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/stat.h>


using namespace std;



/**下载主体入口**/
void* start_down(void* tmp){
	clientConfig* conf = (clientConfig*)tmp;
	if( access((conf->seed_file).c_str(),R_OK) != 0 || conf->page_counter <= 0){
		fprintf(stderr,"[start_down] read seed_file exists,%s\n",(conf->seed_file).c_str());
		return (void*)NULL;
	}

	
	/**初始化下载队列**/
	down_queue _queue;
	
	
	/**启动下载发送请求线程，有多少服务端就启动多少线程**/
	int thread_num = (conf->download_hosts).size();
    down_thread_arg** args = (down_thread_arg**)malloc(sizeof(down_thread_arg*)*thread_num);
    bzero(args,sizeof(down_thread_arg*)*thread_num);
	for(int i = 0; i < thread_num; i++){
        args[i] = new down_thread_arg();
		bzero(args[i]->down_server,30);
		args[i]->q = &_queue;
		strcpy(args[i]->down_server, ((conf->download_hosts)[i%thread_num]).c_str());
		args[i]->conf = conf;
	}
	Thread obj(thread_num);
	obj.createMutilThread(down_func, (void**)args);
	
	
	
	/**开始分配**/
	int down_counter;
	int cnt = 0;
	do{
		down_counter = send_queue(conf, &_queue, cnt++);
		while(_queue.queue_size() > 0){
			sleep(3);
		}
		sleep(5);
	}while( down_counter > 0 || conf->parse_thread_exit <= 0);
	/**parse_thread_exit是标志解析线程是否退出的，解析线程不退出，下载线程不准退出**/
	
	
	/**一切OK，可以让各个下载子线程退出了**/
	_queue.is_finish = 1;
	obj.joinThread();
	
	
	/**释放内存**/
	fprintf(stderr,"[down_func::start_down] free malloc space........\n");
	for(int i = 0; i < thread_num; i++){
        delete args[i];
	}
	free(args);
	
	fprintf(stderr,"[down_func::start_down] download thread start exit...............\n");
	return (void*)NULL;
}



/**执行下载的线程函数**/
void * down_func(void *tmp){
	down_thread_arg* larg = (down_thread_arg*)tmp;
	send_request obj(larg);
	obj.run();
	return (void*)NULL;
}


/**往队列里添加数据**/
int send_queue(clientConfig* conf, down_queue* q, int down_counter){	
	
	/**一些大小及参数限制**/
	int normal_page_size = atoi((conf->normal_page_limit_size).c_str());
	int page_per_proxy = atoi((conf->page_num_per_proxy).c_str());
	if( page_per_proxy <= 0){
		page_per_proxy = 20;
	}
	
	
	string proxy_host;
	
	
	
	/**遍历种子文件**/
	FILE *fp = fopen((conf->seed_file).c_str(),"r");
	if(NULL == fp){
		perror("open seed file failed");
		q->is_finish = 1;
		return -2;
	}
	
	char tmp[4096] = {0};
	map<string, list<string> > data;
	list<string> url_list;
	map<string, list<string> >::iterator iter;
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
		
		/**检查文件是否存在及其大小是否正常**/
		if( access(local_file.c_str(),R_OK) == 0 ){
			struct stat buf; 
			stat(local_file.c_str(),&buf);
			if(buf.st_size <= normal_page_size){
				unlink(local_file.c_str());
			}
			else{
				continue;
			}
		}
		
		/**临时储存起来：代理下载起来比较慢，不能一直用**/
		url_list.push_back(line);
		if(counter > 0 && counter % page_per_proxy == 0){
			proxy_host = "";
			if( down_counter % 2 == 0){
				get_proxy_host(conf, proxy_host);
			}
			data[proxy_host] = url_list;
			q->addQueue(data);
			fprintf(stderr,"[send_queue] add %d elements into downQueue\n",url_list.size());
			data.clear();
			url_list.clear();
		}
		counter++;
	}
	
	if(url_list.size() > 0){
		get_proxy_host(conf, proxy_host);
		data[proxy_host] = url_list;
		q->addQueue(data);
		data.clear();
		url_list.clear();
	}
	fclose(fp);
	return counter;
}


/**获取代理的函数**/
void get_proxy_host(clientConfig* conf, string& h){
	srand(time(NULL));
	h = ((conf->proxy_list).size() <= 0) ? "" : (conf->proxy_list)[rand()%(conf->proxy_list).size()];
}

