#include<stdio.h>
#include<stdlib.h>
#include "threadManage/Queue.h"

//处理任务的总入口函数
//包括检查、下载、解析、校验等

int32_t task_start_second(const string& sf);
int32_t task_start_first(const string& sf);


/**
 * 下载的线程函数
 */
void* download_func(void* q);

/**
 * URL种子分发的线程函数
 */
void* dispatch_download(void* q);


/**
 * 抓取、解析完成后的善后检查函数
 */
int32_t check_download(int filesize);

