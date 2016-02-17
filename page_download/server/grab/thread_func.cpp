#include "grab/thread_func.h"
#include "grab/pageDownload.h"
#include "grab/pageDispatch.h"
#include "threadManage/Thread.h"
#include "utils/Utils.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "utils/ConfigParser.h"
#include "utils/Global.h"
#include "threadManage/Thread.h"

static string seed_file;

int32_t task_start(){
	curl_global_init(CURL_GLOBAL_ALL);

	Queue _queue;
	_queue.init();

    void* _qptr = (void*)&_queue;
	
	///获取种子文件
	global_config.get_item("SEED_FILE_DIR",seed_file);
	Utils::rtrim(seed_file, "/");
	char task_id_tmp[10] = {0};
	sprintf(task_id_tmp, "%d", global_task_id);
	seed_file.append("/").append(task_id_tmp).append(".seed");
	
	///种子分配线程
	Thread pageDis(1);
	pageDis.createMutilThread(dispatch_download, (void**)&_qptr);
	
	//下载线程
	string thread_num_str;
	global_config.get_item("THREAD_NUM_GRAB",thread_num_str);
    int thread_num = atoi(thread_num_str.c_str());
	Thread pageDown(thread_num);
    void** _arg = (void**)malloc(sizeof(void*)*thread_num);
    for(int i = 0; i<thread_num; i++){
        _arg[i] = (void*)&_queue;
    }
	pageDown.createMutilThread(download_func, _arg);


    //join
    pageDis.joinThread();
    pageDown.joinThread();
	
	//free
	pageDis.FreeThread();
	pageDown.FreeThread();
	
    free(_arg);	
	return 0;
}

/**
 * 抓取前的检查函数
 */
int32_t check_start(){

	return 0;
}

/**
 * 下载的线程函数
 */
void* download_func(void* _q){
    Queue* q = (Queue*)_q;
    pageDownload down(q);
    down.run();
    return ((void*)0);
}

/**
 * URL种子分发的线程函数
 */
void* dispatch_download(void* _q){
    Queue* q = (Queue*)_q;
    pageDispatch dispatch(q, seed_file);
    dispatch.run();
    return ((void*)0);
}

/**
 * 页面解析线程函数
 */
void* parse_func(void* _q){
    Queue* q = (Queue*)_q;
    return ((void*)0);
}

/**
 * 页面解析分发线程函数
 */
void* dispatch_parse(void* _q){
    Queue* q = (Queue*)_q;
    return ((void*)0);
}

/**
 * 检查种子是否被下载完
 */
int32_t check_download(){

	int not_num = 0;
	//检查种子文件里的是否被下载完
	string tmp_file = seed_file;
	tmp_file.append(".tmp");
	FILE* fp_tmp = fopen(tmp_file.c_str(),"a+");
	FILE *fp_seed = fopen(seed_file.c_str(),"r");
	char tmp[2048] = {0};
	while(!feof(fp_seed)){
		fgets(tmp,2048,fp_seed);
		string line(tmp);
		Utils::trim(line,"\t\n ");
		int pos = line.find('\t');
		if(pos == string::npos){
			continue;
		}
		//已存在
		vector<string> tmp_line;
		Utils::explode(line, "\t", tmp_line);
		if (tmp_line.size() < 3){
			continue;
		}
        string file = tmp_line[2];
		if( access(file.c_str(),R_OK) != 0 ){
			line.append("\n");
			fputs(line.c_str(), fp_tmp);
			not_num ++;	
			fprintf(stderr, "download failed: %s\n", line.c_str());
		}
		bzero(tmp,2048);
	}
	fclose(fp_seed);
	fclose(fp_tmp);

	//互换文件名
	if(not_num > 0){
		string tmp = tmp_file;
		tmp.append(".tmmmmmp");
		rename(seed_file.c_str(),tmp.c_str());
		rename(tmp_file.c_str(), seed_file.c_str());
		rename(tmp.c_str(), tmp_file.c_str());
		return not_num;
	}

	unlink(tmp_file.c_str());
	return 0;
}

