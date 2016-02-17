#include "down_parse_client_serviceHandler.h"
#include "down_client/down_func.h"
#include "parse_client/parse_func.h"
#include "utils/Thread.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/file.h>
#include<fcntl.h>


typedef struct func_arg{
	int32_t task_id;
	int32_t is_retry;
	char conf_file[255];
}func_arg;


/**处理任务的方法**/
void* task_func(void* arg){
	func_arg* param = (func_arg*)arg;
	
	
	/**解析配置**/
	char tmp[10] = {0};
	sprintf(tmp,"%d",param->task_id);
	clientConfig conf(param->conf_file,tmp);
	conf.is_retry = param->is_retry;
	free(param);
	if(access((conf.seed_file).c_str(), R_OK) == 0){
		int fd = open((conf.seed_file).c_str(), O_WRONLY);
		if( fd <= 0 ){
			perror("open seed file failed\n");
			return (void*)NULL;
		}
		if( flock(fd, LOCK_EX|LOCK_NB) < 0){
			fprintf(stderr, "[task_func] lock seed file failed:%s\n",(conf.seed_file).c_str());
			return (void*)NULL;
		}
		flock(fd, LOCK_UN);
		close(fd);
	}
	
	
	
	/**生成一些配置信息**/
	conf.generateInfo();
	conf.update_proxy_list();
	conf.printConf();
	clientConfig* conf_ptr = &conf;
	
	
	
	if(access((conf_ptr->task_dir).c_str(), W_OK) != 0){
		return (void*)NULL;
	}
	if( (conf_ptr->engine_info).size() <= 0){
		return (void*)NULL;
	}
	if(access((conf_ptr->seed_file).c_str(), R_OK) != 0){
		return (void*)NULL;
	}
	
	/**lock seed_file**/
    int fd = open((conf_ptr->seed_file).c_str(), O_WRONLY);
	if( fd <= 0 ){
		perror("open seed file failed\n");
		return (void*)NULL;
	}
	if( flock(fd, LOCK_EX|LOCK_NB) < 0){
		fprintf(stderr, "[task_func] lock seed file failed:%s\n",(conf_ptr->seed_file).c_str());
		return (void*)NULL;
	}
	
	
	/**抓取中**/
	conf_ptr->set_task_status("2");
	
	
	Thread down_obj(1);
	down_obj.createMutilThread(start_down, (void**)&conf_ptr);
	
	Thread parse_obj(1);
	parse_obj.createMutilThread(start_parse, (void**)&conf_ptr);
	

	parse_obj.joinThread();
	down_obj.joinThread();
	
	
	/**最后入库**/
	(conf_ptr->in_db_item).clear();
	conf_ptr->import_db();
	
	
	/**已抓取**/
	fprintf(stderr,"\n[task_func] Everything is OK, task_id[%s] ,prepare exit........Bye\n",(conf_ptr->task_id).c_str());
	conf_ptr->set_task_status("3");
	flock(fd, LOCK_UN);
	close(fd);
	unlink((conf_ptr->seed_file).c_str());
	
	
	fprintf(stderr,"[task_func] *********************  task_id[%s]  END  **************************\n", \
					(conf_ptr->task_id).c_str());
	return (void*)NULL;
}

int32_t down_parse_client_serviceHandler::ping(){
	return 0;
}


/**
 * 开始处理任务的抓取及解析处理
 *
 * @param	int32_t	 task_id	任务编号
 * @param	int32_t  is_retry	是否重试，其取值如下：
 *			0：新抓取
 *			1：完全重试（删掉以前的）
 *			2：部分重试（只重试未解析成功的部分）
 */
int32_t down_parse_client_serviceHandler::start(const int32_t task_id, const int32_t is_retry){
	if( task_id <= 0){
		fprintf(stderr,"[down_parse_client_serviceHandler::start] invalid task_id");
		return -3;
	}

	func_arg* arg = (func_arg*)malloc(sizeof(func_arg));
	bzero(arg,sizeof(func_arg));
	arg->task_id = task_id;
	arg->is_retry = is_retry;
	strcpy(arg->conf_file,conf_file.c_str());
	
	Thread obj(1);
	obj.createMutilThread(task_func, (void**)&arg);
	
	return 0;
}

