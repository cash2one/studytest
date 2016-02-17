#ifndef __THREAD_H__
#define __THREAD_H__
#include <pthread.h>
#include <stdint.h>

class Thread{

public:
	Thread(){}
	~Thread();
	Thread(int thread_counter) : thread_num(thread_counter), thread_handle(NULL){}
	
	///创建指定数量个线程
	int32_t  createMutilThread(void *(*pfn)(void *), void **vpArg);
	
	///join所有线程，当前线程会一直阻塞到指定线程结束
	int32_t joinThread();
	
	///释放所有线程
	void FreeThread();
	
	///设置要创建的线程的数量
	void set_thread_num(int num);
	
	pthread_t *thread_handle; //线程句柄集合

	
private:
	uint32_t thread_num; //线程个数
	
	///初始化成员变量
	int32_t init();
	
	
};
#endif
