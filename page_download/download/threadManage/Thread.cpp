#include "threadManage/Thread.h"
#include<stdio.h>
#include<stdlib.h>

/**
 * 创建指定数量的线程
 *
 * @param	pfn		线程要执行的函数
 * @param	arg 	线程的参数列表
 */
int32_t  Thread::createMutilThread(void *(*pfn)(void *), void **arg){
	if(init() != 0 ){
		fprintf(stderr, "create mutil thread failed\n");
		return -1;
	}
	
	///循环创建线程
	for(int i = 0; i < thread_num; i++){
		void *a = (NULL == arg) ? NULL : arg[i];
		if( pthread_create(&(thread_handle[i]), NULL,  pfn, a) != 0){
			perror("create thread failed");
			return -2;
		}
	}
	
	return 0;
}

/**
 * join所有的线程，当前线程会被阻塞到指定线程执行完毕后才继续
 *
 * @param	thread_ret	 线程的返回值的集合
 */
int32_t Thread::joinThread(){
	for(int i = 0; i < thread_num; i++){
		if(pthread_join(thread_handle[i], NULL) != 0){
			fprintf(stderr, "join %dth thread failed\n", i);
		}
	}
	return 0;
}

/**
 * 结束并释放所有线程
 */
void Thread::FreeThread(){

}

/**
 * 初始化一些成员变量
 */
int32_t Thread::init(){
	if( thread_num <= 0 || NULL != thread_handle){
		return -1;
	}
	thread_handle = new pthread_t[thread_num];
	return 0;
}

Thread::~Thread(){
	delete[] thread_handle;
	thread_handle = NULL;
	thread_num = 0;
}


