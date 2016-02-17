#ifndef _PARSE_QUEUE_H__
#define _PARSE_QUEUE_H__

#include<list>
#include<stdint.h>
#include<pthread.h>
#include<string>


/**
 * 基于list的互斥队列，以mutex加锁
 */
class parse_queue{

public:
	parse_queue();
	
	
	/**一些初始化**/
	int32_t init();
	
	
	//向队列中添加数据
	int32_t addQueue( std::list<std::string>& data);
	
	
	//从队列里获取数据
	int32_t getQueue(uint32_t num, std::list<std::string>& data);
	
	
	///获取队列当前的大小
	uint32_t queue_size();
	
	
	/**是否可以退出的标志**/
	int is_finish;

private:
	const static uint32_t MAX_QUEUE_SIZE = 50;
	std::list<std::string> _queue; //队列
	
	pthread_mutex_t mutex; //互斥锁
	pthread_cond_t empty_cond; //为空时的条件变量
	pthread_cond_t full_cond; //满了时的条件变量
	
};

#endif


