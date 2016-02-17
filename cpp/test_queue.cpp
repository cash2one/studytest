#include "test_queue.h"
#include<iostream>

using namespace std;

test_queue::test_queue(){

}
int test_queue::run(){
    //queue<string> _queue;
    _queue.push("wendao");
    _queue.push("tencent");
    _queue.push("sogou");
    _queue.push("baidu");
    cout<<_queue.front()<<endl; //wendao
    cout<<_queue.back()<<endl; //baidu
    cout<<_queue.size()<<endl; //4
    cout<<_queue.empty()<<endl; //0
    _queue.pop();
    cout<<_queue.front()<<endl; //tencent
    cout<<_queue.back()<<endl; //baidu
    return 0;
}
