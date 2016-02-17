#include "test_class.h"
#include<iostream>
#include<map>

using namespace std;

int test_class::i = 50;
test_class::test_class(){
    cout<<"call default contructor"<<endl;
}
test_class::test_class(const test_class &inst){
    this->num = inst.num + 1;
    cout<<"copy constructor"<<endl;
}
test_class& test_class::operator=(const test_class &inst){
    cout<<"call operator constructor"<<endl;
    this->num = inst.num + 500;
}
test_class::~test_class(){
    cout<<"call deconstructor"<<endl;
}
void test_class::show(){
    cout<<"static i = " <<i<<endl;
}
test_class::operator int(){
    return 123456;
}
void test_class::showAll() const {
    cout<<"-----------------------------"<<endl;
    cout<<"this->num="<<this->num<<endl;
    cout<<"test_class::i="<<test_class::i<<endl;
    cout<<"(*this).i="<<(*this).i<<endl;
    cout<<"-----------------------------"<<endl;
}

test_class & test_class::operator+=(const test_class &inst){
    cout<<"call test_class::operator+="<<endl;
    this->num = inst.num * 10;
    return (*this);
}

void test_class::printStr(){
    cout<<"virtual function of test_class"<<endl;
    this->num = 100;
}
void test_class::printTest(){
    cout<<"test_class printTest"<<endl;
    map<string,string> _map;
    _map.insert(make_pair("wendao","liuyongshuai"));
    _map.insert(make_pair("tencent","baidu"));
    throw _map;
}
