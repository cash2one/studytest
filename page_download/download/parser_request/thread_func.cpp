#include "threadManage/Thread.h"
#include "threadManage/Queue.h"
#include "mysql/mark_oss.h"
#include "mysql/DB.h"
#include "parser_request/thread_func.h"
#include "parser_request/doParse.h"
#include "parser_request/parserDispatch.h"
#include "parser_request/parserConfig.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static string seed_file;
static parserConfig* conf;
static Queue _queue;


//线程管理用锁及记录标示
static int global_counter;
pthread_mutex_t global_mutex;
pthread_cond_t global_cond;


///开始入口函数
int32_t start(const string& seed, const string& config){
	//初始化全局变量
	seed_file = seed;
	parserConfig c(config, seed_file);
	c.page_limit_size = 100000;
	conf = &c;
	
	global_counter = 0;
    conf->print_conf();
	
	//获取引擎列表
    if(get_engine_list() != 0){
        fprintf(stderr,"**************get engine list info failed, exit........***********\n\n");
        return -3;
    }

	//初始化队列类
	_queue.init();
	
	//启动分配线程
	Thread parserDis(1);
	do_global_counter(1);
	parserDis.createMutilThread(dispatch_func, (void**)NULL);
	
	//有多少server端就启动多少解析线程，一个对一个
	int parser_num = (conf->hosts).size();
	Thread** pa_thread = (Thread**)malloc(sizeof(Thread*)*parser_num);
	bzero(pa_thread,sizeof(Thread*)*parser_num);
	int** cnt = (int**)malloc(sizeof(int*)*parser_num);
	bzero(cnt,sizeof(int*)*parser_num);
	for(vector<string>::size_type i=0; i<parser_num; i++){
		cnt[i] = new int(i);;
		pa_thread[i] = new Thread(1);
		pa_thread[i]->createMutilThread(parser_func, (void**)&(cnt[i]));
	}
	
	//检查所有的线程是否都结束了
	pthread_mutex_lock(&global_mutex);
	while(global_counter > 0){
		pthread_cond_wait(&global_cond, &global_mutex);
	}
	pthread_mutex_unlock(&global_mutex);
	pthread_cond_broadcast(&global_cond);
	
	//清理动态分配的变量
	for(int i =0; i < parser_num; i++){
		pthread_cancel((pa_thread[i]->thread_handle)[0]);
		delete pa_thread[i];
		delete cnt[i];
	}
	free(pa_thread);
	free(cnt);
	return 0;
}

//解析线程执行函数
void* parser_func(void* n){
	int num = *((int*)n);
	doParse obj((conf->hosts)[num], &_queue, conf);
	obj.run();
	return 0;
}

//分配线程执行函数
void* dispatch_func(void *arg){
	parserDispatch obj(&_queue, seed_file, conf);
	obj.run();
	do_global_counter(-1);
	return 0;
}

//增减全局计数器
void do_global_counter(int num){
	pthread_mutex_lock(&global_mutex);
	global_counter += num;
	fprintf(stderr,"[thread_func] current thread num %d\n",global_counter);
	pthread_mutex_unlock(&global_mutex);
	pthread_cond_broadcast(&global_cond);
}


//查询任务信息，取出引擎信息
int get_engine_list(){
	mark_oss db(conf->mysql_host, 
				conf->mysql_user, 
				conf->mysql_db, 
				conf->mysql_password, 
				conf->mysql_charset,
				atoi((conf->mysql_port).c_str())
	);
	
	//开始查询任务信息
	map<string,string> task_info;
	db.get_task_info(conf->task_id, task_info);
	if( task_info.size() <= 0){
		fprintf(stderr,"[thread_func] get task info failed,task_id=%d\n",conf->task_id);
		return -1;
	}
	
	//查询引擎信息
	vector<string> eid;
	Utils::explode(task_info["engine"], "\t", eid);
	map<string,vector<string> > where;
	where["eid:in"] = eid;
	map<string, string> orderby;
	map<string, map<string, string> > engine_list;
	db.getList("tb_engine", where, 1, eid.size(), "eid", orderby, engine_list);
	if( engine_list.size() <= 0){
		fprintf(stderr,"[thread_func] get engine list failed,engine ids %s\n",(task_info["engine"]).c_str());
		return -1;
	}
	
	//格式化结果
	map<string, map<string, string> >::iterator iter;
	for(iter=engine_list.begin(); iter!=engine_list.end(); iter++){
		string ename = (iter->second)["name"];
		string eid_str = (iter->second)["eid"];
		//sogou、 baidu、360一个一个的判断
		if(ename.find("sogou") != string::npos){
			(conf->engine_info)[eid_str] = (conf->need_ad == 0) ? "sogou-no-ad" : "sogou";
		}
		else if(ename.find("baidu") != string::npos){
			(conf->engine_info)[eid_str] = "baidu";
		}
		else if(ename.find("360") != string::npos){
			(conf->engine_info)[eid_str] = "360so";
		}
		else{
			fprintf(stderr, "[thread_func] unknown engine name %s", ename.c_str());
		}
	}
	return 0;
}





