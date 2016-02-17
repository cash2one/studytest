#include "down/thread_func.h"
#include "down/pageDownload.h"
#include "down/pageDispatch.h"
#include "threadManage/Thread.h"
#include "utils/Utils.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static string seed_file;
static vector<string> proxy;

//线程管理用锁及记录标示
int global_counter;
pthread_mutex_t global_mutex;
pthread_cond_t global_cond;


//遍历文件获取引擎列表
static int dispath_url_list(const string& f, map<string, list<string> >& engines ){
	FILE *fp = fopen(f.c_str(),"r");
	char tmp[4096] = {0};
	while(!feof(fp)){
		fgets(tmp, 4096, fp);
		if(strlen(tmp) <= 0){
			continue;
		}
        string line(tmp);
        Utils::trim(line,"\t\n ");
        int pos  = line.find('\t');
        if(pos == string::npos ){
            continue;
        }
        //已存在
		vector<string> tmp_line;
		Utils::explode(line, "\t", tmp_line);
		if (tmp_line.size() < 2){
			continue;
		}
		string url = tmp_line[0];
        string file = tmp_line[1];
        if( access(file.c_str(),R_OK) == 0){
            continue;
        }
		file.append("\t").append(url);
		
		//根据url的host分类
		vector<string> h;
		Utils::explode(url,"/",h);
		if(h.size() < 3){
			continue;
		}
		string host = h[2];
		if(host.length() < 5){
			continue;
		}
		if(engines.find(host) == engines.end()){
			list<string> tl;
			tl.push_back(file);
			engines.insert(make_pair(host,tl));
		}
		else{
			engines[host].push_back(file);
		}

		bzero(tmp,4096);
	}
	fclose(fp);
	return 0;
}

int32_t task_start_second(const string& sf){
	proxy.push_back("adslspider01.web.zw.vm.sogou-op.org:8080");
	//proxy.push_back("adslspider02.web.zw.vm.sogou-op.org:8080");
	//proxy.push_back("adslspider03.web.zw.vm.sogou-op.org:8080");
	//proxy.push_back("adslspider04.web.zw.vm.sogou-op.org:8080");
	map<string, list<string> > engines;
	dispath_url_list(seed_file,engines);
	if( engines.size() <= 0){
		return -1;
	}

	///每个引擎一个队列
	int eng_size = engines.size();
	Queue** _queue = (Queue**)malloc( sizeof(Queue*) * eng_size );
	bzero(_queue, sizeof(Queue*) * eng_size);
	map<string, list<string> >::iterator iter;
	int i=0;
	for(iter=engines.begin(); iter != engines.end(); iter++){
		fprintf(stderr ,"host: %s\n", (iter->first).c_str());
		_queue[i] = new Queue;
		_queue[i]->init();
		_queue[i]->addQueue(iter->second);
		_queue[i]->queue_index = i++;
	}
	
	global_counter = 0;
	
	//每个队列一个分配管理线程
	Thread** pdis = (Thread**)malloc( sizeof(Thread*) * eng_size );
	bzero(pdis, sizeof(Thread*) * eng_size);
	for(int i=0; i< eng_size; i++){
		pthread_mutex_lock(&global_mutex);
		global_counter ++;
		fprintf(stderr,"lock mutex,current counter=%d\n",global_counter);
		pthread_mutex_unlock(&global_mutex);
		pthread_cond_broadcast(&global_cond);
		pdis[i] = new Thread(1);
		pdis[i]->createMutilThread(dispatch_download, (void**)&(_queue[i]));
	}

	///每个队列，有多少个代理就再分几个线程
	int proxy_size = proxy.size();
	Thread **pdown = (Thread**)malloc( sizeof(Thread*) * eng_size * proxy_size );
	bzero(pdown, sizeof(Thread*) * eng_size * proxy_size);
	for(int i=0; i< eng_size; i++){
		for(int j=0; j < proxy_size; j++){
			int num = i * eng_size + j;
			pthread_mutex_lock(&global_mutex);
			global_counter ++;
			fprintf(stderr,"lock mutex,current counter=%d\n",global_counter);
			pthread_mutex_unlock(&global_mutex);
			pthread_cond_broadcast(&global_cond);
			pdown[num] = new Thread(1);
			pdown[num]->createMutilThread(download_func, (void**)&(_queue[i]));
		}
	}

	///比较奇葩的判断线程是否都结束的方法
	pthread_mutex_lock(&global_mutex);
	while(global_counter > 0){
		fprintf(stderr,"pthread_cond_wait, current global_counter=%d\n",global_counter);
		pthread_cond_wait(&global_cond, &global_mutex);
	}
	pthread_mutex_unlock(&global_mutex);
	pthread_cond_broadcast(&global_cond);
	
	for(int i=0; i< eng_size; i++){
		delete pdis[i];
		delete _queue[i];
	}
	for(int i=0; i< eng_size; i++){
		for(int j=0; j < proxy_size; j++){
			int num = i * eng_size + j;
			delete pdown[num];
		}
	}
	free(_queue);
	free(pdis);
	free(pdown);
	
	return 0;
}

///暴力下载小神童
int32_t task_start_first(const string& sf){
	curl_global_init(CURL_GLOBAL_ALL);
	seed_file = sf;
	
	///实例化全局锁
	if( pthread_mutex_init(&global_mutex, NULL) != 0){
		fprintf(stderr,"init global mutex failed\n");
		return -1;
	}
	if( pthread_cond_init(&global_cond, NULL) != 0 ){
		fprintf(stderr,"init global cond failed\n");
		return -3;
	}
	global_counter = 0;
	
	Queue _queue;
	_queue.init();

    void* _qptr = (void*)&_queue;
	
	///种子分配线程
	Thread pageDis(1);
	global_counter ++;
	pageDis.createMutilThread(dispatch_download, (void**)&_qptr);
	sleep(5);
	
	//下载线程
	int thread_num = 30;
	
	pthread_mutex_lock(&global_mutex);
	global_counter += thread_num;
	fprintf(stderr,"counter + %d\n",thread_num);
	pthread_mutex_unlock(&global_mutex);
	pthread_cond_broadcast(&global_cond);
	
	fprintf(stderr,"prepare start pthread num %d\n",thread_num);
	Thread pageDown(thread_num);
    void** _arg = (void**)malloc(sizeof(void*)*thread_num);
    for(int i = 0; i<thread_num; i++){
        _arg[i] = (void*)&_queue;
    }
	pageDown.createMutilThread(download_func, _arg);


	///比较奇葩的判断线程是否都结束的方法
	pthread_mutex_lock(&global_mutex);
	while(global_counter > 0){
		pthread_cond_wait(&global_cond, &global_mutex);
	}
	pthread_mutex_unlock(&global_mutex);
	pthread_cond_broadcast(&global_cond);
	
    free(_arg);	
	return 0;
}

/**
 * 下载的线程函数
 */
void* download_func(void* _q){
    Queue* q = (Queue*)_q;
	string p = "";
	if( q->queue_index >= 0){
		p.append(proxy[0]);
	}
    pageDownload down(q,p);
    down.run();
	pthread_mutex_lock(&global_mutex);
	fprintf(stderr,"download_func finish,lock mutex\n");
	global_counter --;
	pthread_mutex_unlock(&global_mutex);
	pthread_cond_broadcast(&global_cond);
    return ((void*)0);
}

/**
 * URL种子分发的线程函数
 */
void* dispatch_download(void* _q){
    Queue* q = (Queue*)_q;
    pageDispatch dispatch(q, seed_file);
	if( q->queue_index < 0){
		dispatch.run();
	}
	pthread_mutex_lock(&global_mutex);
	fprintf(stderr,"dispatch_download finish,lock mutex\n");
	global_counter --;
	pthread_mutex_unlock(&global_mutex);
	pthread_cond_broadcast(&global_cond);
    return ((void*)0);
}

/**
 * 检查种子是否被下载完
 */
int32_t check_download(int filesize){

	struct stat buf; 
	int not_num = 0;
	//检查种子文件里的是否被下载完
	char tmp[4096] = {0};
	FILE *fp_seed = fopen(seed_file.c_str(),"r");
	while(!feof(fp_seed)){
		fgets(tmp,4096,fp_seed);
		string line(tmp);
		Utils::trim(line,"\t\n ");
		int pos = line.find('\t');
		if(pos == string::npos){
			continue;
		}
		//已存在
		vector<string> tmp_line;
		Utils::explode(line, "\t", tmp_line);
		if (tmp_line.size() < 2){
			continue;
		}
        string file = tmp_line[1];
		if( access(file.c_str(),R_OK) != 0 ){
			not_num ++;	
			fprintf(stderr, "retry download: %s\n", file.c_str());
			continue;
		}
		//文件大小为0的删除
		stat(file.c_str(),&buf);
		if(buf.st_size <= filesize){
			not_num++;
			unlink(file.c_str());
		}
		bzero(&buf,sizeof(buf));
		bzero(tmp,4096);
	}
	fclose(fp_seed);
	return not_num;
}

