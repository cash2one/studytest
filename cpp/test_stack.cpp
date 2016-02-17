#include "test_stack.h"
#include<iostream>

using namespace std;

test_stack::test_stack(){

}

int test_stack::run(){
    //stack<string> _stack;
    _stack.push("wendao");
    _stack.push("liuyongshuai");
    _stack.push("tencent");
    cout<<_stack.top()<<endl; //tencent
    cout<<_stack.size()<<endl; //3
    cout<<_stack.empty()<<endl; //0
    _stack.pop();
    cout<<_stack.top()<<endl; //liuyongshuai
    return 0;
}
