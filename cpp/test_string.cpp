#include<cstdio>
#include<iostream>
#include "test_string.h"
using namespace std;
test_string::test_string(char *str){
    test_str = string(str);
}
test_string::test_string(string str){
    test_str = str;
}
int test_string::run(){
    cout<<"origin string : "<<test_str<<endl;
    cout<<"str.push_back('Y')"<<endl;
    test_str.push_back('Y');
    cout<<"str.length()="<<test_str.length()<<endl;
    cout<<"str.size()="<<test_str.size()<<endl;
    cout<<"str.max_size()="<<test_str.max_size()<<endl;
    cout<<"str.find(\"name\",0)="<<test_str.find("name",0)<<endl;
    cout<<"str.rfind(\"name\",0)="<<test_str.rfind("name",0)<<endl;
    cout<<"str.find_first_of(\"liu\",0)="<<test_str.find_first_of("liu",0)<<endl;
    cout<<"str.find_last_of(\"name\",0)="<<test_str.find_last_of("name",0)<<endl;
    cout<<"str.find_first_not_of(\"age\",0)="<<test_str.find_first_not_of("age",0)<<endl;
    cout<<"str.find_last_not_of(\"name\",0)="<<test_str.find_last_not_of("name",0)<<endl;
    cout<<"str.replace(5,5,\"---\")="<<test_str.replace(5,5,"---")<<endl;
    cout<<"str.substr(4,9=)"<<test_str.substr(4,9)<<endl;
    cout<<"str.insert(9,\"VVVV\")="<<test_str.insert(9,"VVVV")<<endl;
    cout<<"str.empty()="<<test_str.empty()<<endl;
    cout<<"str.data()="<<test_str.data()<<endl;
    cout<<"str.c_str()="<<test_str.c_str()<<endl;
    cout<<"str.capacity()="<<test_str.capacity()<<endl;
    cout<<"str.reserve(0)"<<endl;
    test_str.reserve(0);
    cout<<"str.capacity()="<<test_str.capacity()<<endl;
    cout<<"str.resize(0)"<<endl;
    test_str.resize(0);
    cout<<"str.capacity()="<<test_str.capacity()<<endl;
    cout<<"str.compare(\"bjtuwendao\")="<<test_str.compare("bjtuwendao")<<endl;
    cout<<"str.append(\"LLLLL\")="<<test_str.append("LLLLL")<<endl;
    cout<<"str.assign(\"liuyongshuai@hotmail.com\")="<<test_str.assign("liuyongshuai@hotmail.com")<<endl;
    cout<<"str.at(10)="<<test_str.at(10)<<endl;

    return 0;
}
