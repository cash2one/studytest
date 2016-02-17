#include<map>
#include<iostream>
#include<string>
#include "test_map.h"

using namespace std;

test_map::test_map(){
    _map = map<string,int>();
}
int test_map::run(){
    _map.insert(pair<string,int>("wendao",1));
    _map.insert(make_pair("sina",45));
    _map["baidu"]=1111;
    if(_map["abc"] == 1){
        cout<<"000000000000000000"<<endl;
    }
    _map.insert(map<string,int>::value_type("tencent",633));
    map<string,int>::iterator iter;
    for(iter=_map.begin(); iter != _map.end(); iter++){
        cout<<"iter->first="<<iter->first<<"\titer->second="<<iter->second<<endl;
    }
    cout<<"map.size()="<<_map.size()<<endl;
    iter = _map.find("baidu");
    if(iter != _map.end()){
        cout<<iter->first<<"\t"<<iter->second<<endl;
    }
    return 0;
}
