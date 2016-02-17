#ifndef __QUEUE_H__
#define __QUEUE_H__

#include<list>
#include<stdint.h>
#include<pthread.h>
#include<string>

using namespace std;

/**
 * 基于list的互斥队列，以mutex加锁
 */
class Queue{

public:
	Queue():queue_index(-1){}
	int32_t init();
	
	//向队列中添加数据
	int32_t addQueue( const list<string>& data);
	
	//从队列里获取数据
	int32_t getQueue(uint32_t num, list<string>& data);
	
	///获取队列当前的大小
	uint32_t size();
	
	//序号
	int32_t queue_index;

private:
	const static uint32_t MAX_QUEUE_SIZE = 1000;
	list<string> _queue; //队列
	
	pthread_mutex_t mutex; //互斥锁
	pthread_cond_t empty_cond; //为空时的条件变量
	pthread_cond_t full_cond; //满了时的条件变量
	
};

#endif


