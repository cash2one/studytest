#include "parse_queue.h"
#include<string.h>
#include<time.h>
#include<errno.h>

using namespace std;


///超时3秒
static struct timespec tmp = {1, 0};



/**构造函数**/
parse_queue::parse_queue():is_finish(0){
	init();
}



/**初始化**/
int32_t parse_queue::init(){
	if( pthread_mutex_init(&mutex, NULL) != 0){
		return -1;
	}
	if( pthread_cond_init(&empty_cond, NULL) != 0 ){
		return -2;
	}
	if( pthread_cond_init(&full_cond, NULL) != 0 ){
		return -3;
	}
	
	_queue.clear();
	return 0;
}



/**往队列中添加数据**/
int32_t parse_queue::addQueue( list<string>& data){
	if( data.empty()){
		return -1;
	}
	
	///加锁
	pthread_mutex_lock(&mutex);
	
	//队列是否已满
	while( data.size() + _queue.size() > MAX_QUEUE_SIZE){
		pthread_cond_timedwait(&full_cond, &mutex, &tmp);
	}
	
	///合并两个列表
	_queue.splice(_queue.begin(), data);
	pthread_mutex_unlock(&mutex);
	pthread_cond_broadcast(&full_cond);
	pthread_cond_broadcast(&empty_cond);
	return 0;
}



/**提取数据，指定数量**/
int32_t parse_queue::getQueue(uint32_t num, list<string>& data){
	if( num <= 0 ){
		return -1;
	}
    data.clear();
	

	int counter = 0;
	
	//加锁
	pthread_mutex_lock(&mutex);
	while( _queue.empty()){
		if( counter++ >= 3 ){
			break;
		}
		pthread_cond_timedwait(&empty_cond,&mutex,&tmp);
	}


    list<string>::iterator iter = _queue.begin();
	if( _queue.size() <= num){
		data.splice(data.begin(), _queue);
	}
	else{
		for(int i=1; i<num;i++){
			iter ++;
		}
		data.splice(data.begin(), _queue,_queue.begin(), iter);
	}
	

	
	pthread_mutex_unlock(&mutex);
	pthread_cond_broadcast(&full_cond);
	pthread_cond_broadcast(&empty_cond);
	
	return 0;
}

///获取队列当前的大小
uint32_t parse_queue::queue_size(){
	return _queue.size();
}
