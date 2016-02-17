#include<stdio.h>
#include<stdlib.h>
#include "threadManage/Queue.h"

//处理任务的总入口函数
//包括检查、下载、解析、校验等
int32_t task_start();

/**
 * 抓取进程开始前的检查函数，主要功能：
 * 避免重复抓取(同一个任务只能同时抓取一次)
 * 若抓取的是已处理过的任务则要新开一个子任务
 */
int32_t check_start();

/**
 * 下载的线程函数
 */
void* download_func(void* q);

/**
 * URL种子分发的线程函数
 */
void* dispatch_download(void* q);

/**
 * 页面解析线程函数
 */
void* parse_func(void* q);

/**
 * 页面解析分发线程函数
 */
void* dispatch_parse(void* q);

/**
 * 抓取、解析完成后的善后检查函数
 */
int32_t check_download();

