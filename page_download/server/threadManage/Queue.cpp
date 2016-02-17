#include "threadManage/Queue.h"
#include<string.h>
#include<time.h>
#include<errno.h>

///超时3秒
static struct timespec tmp = {3, 0};
int32_t Queue::init(){
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

//向队列中添加数据
int32_t Queue::addQueue( const list<string>& data){
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
	list<string>::const_iterator iter;
	for(iter=data.begin(); iter!=data.end(); iter++){
		_queue.push_back(*iter);
	}
	_queue.insert(_queue.end(), data.begin(), data.end());
	pthread_mutex_unlock(&mutex);
	if( _queue.size() > 0 ){
		pthread_cond_broadcast(&empty_cond);
	}
	return 0;
}

//从队列里获取数据
int32_t Queue::getQueue(uint32_t num, list<string>& data){
	if( num <= 0 ){
		return -1;
	}
    data.clear();
	

	int counter = 0;
	
	//加锁
	pthread_mutex_lock(&mutex);
	while( _queue.size() <= 0){
		counter++;
		if( counter >= 3 ){
			break;
		}
		pthread_cond_timedwait(&empty_cond,&mutex,&tmp);
	}
	
	int cnt = 1;
	while( cnt <= num && _queue.size() > 0){
		data.push_back(_queue.front());
		_queue.pop_front();
		cnt ++;
	}
	
	pthread_mutex_unlock(&mutex);
	pthread_cond_broadcast(&full_cond);
	pthread_cond_broadcast(&empty_cond);
	
	return 0;
}

///获取队列当前的大小
uint32_t Queue::size(){
	return _queue.size();
}
