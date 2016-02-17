#ifndef __DOWN_QUEUE_H__
#define __DOWN_QUEUE_H__

#include<list>
#include<map>
#include<stdint.h>
#include<pthread.h>
#include<string>


/**
 * 基于list的互斥队列，以mutex加锁
 */
class down_queue{

public:
	down_queue();
	
	
	/**初始化**/
	int32_t init();
	
	
	/**向队列中添加数据**/
	int32_t addQueue( const std::map<std::string, std::list<std::string> >& data );
	
	
	/**从队列里提取数据**/
	int32_t getQueue(std::map<std::string, std::list<std::string> >& data);
	
	
	/**当前队列的大小**/
	uint32_t queue_size();
	
	
	/**各线程是否可以结束的标志**/
	int is_finish;

private:

	/**队列大小限制**/
	const static uint32_t MAX_QUEUE_SIZE = 5;
	
	
	/**队列容器：里面的元素是map，其key是proxy_host，value是要下载的url及其位置的列表**/
	std::list< std::map<std::string, std::list<std::string> > > _queue;
	
	
	/**一些互斥锁及条件变量**/
	pthread_mutex_t mutex; //互斥锁
	pthread_cond_t empty_cond; //为空时的条件变量
	pthread_cond_t full_cond; //满了时的条件变量
	
};

#endif


