#include "down_queue.h"
#include<string.h>
#include<time.h>
#include<errno.h>

using namespace std;


///超时3秒
static struct timespec tmp = {1, 0};


/**构造函数**/
down_queue::down_queue():is_finish(0){
	init();
}


/**初始化互斥变量**/
int32_t down_queue::init(){
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



/**向队列中添加数据**/
int32_t down_queue::addQueue( const map<string, list<string> >& data){
	if( data.empty()){
		return -1;
	}
	
	///加锁
	pthread_mutex_lock(&mutex);
	
	//队列是否已满
	while( data.size() + _queue.size() > MAX_QUEUE_SIZE){
		pthread_cond_timedwait(&full_cond, &mutex, &tmp);
	}
	
	
	_queue.push_back(data);
		

	pthread_mutex_unlock(&mutex);
	pthread_cond_broadcast(&empty_cond);
	pthread_cond_broadcast(&full_cond);
	return 0;
}



/**从队列里提取一个元素**/
int32_t down_queue::getQueue(map<string, list<string> >& data){
    data.clear();
	

	//加锁
	int counter = 0;
	pthread_mutex_lock(&mutex);
	while( _queue.size() <= 0){
		counter++;
		if( counter >= 3 ){
			break;
		}
		pthread_cond_timedwait(&empty_cond,&mutex,&tmp);
	}
	

	/**提取数据**/
	map<string, list<string> >::iterator iter;
	while( _queue.size() > 0){
		map<string, list<string> > tmp_ele = _queue.front();
		for(iter=tmp_ele.begin();  iter!=tmp_ele.end();  iter++){
			data.insert(pair<string,list<string> >(iter->first,iter->second));
		}
		_queue.pop_front();
	}
	
	pthread_mutex_unlock(&mutex);
	pthread_cond_broadcast(&full_cond);
	pthread_cond_broadcast(&empty_cond);
	
	return 0;
}



/**返回队列大小**/
uint32_t down_queue::queue_size(){
	return _queue.size();
}


