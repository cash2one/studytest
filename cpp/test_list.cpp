#include<list>
#include<string>
#include<iostream>

#include "test_list.h"

using namespace std;

test_list::test_list(){

}
int test_list::run(){
    //list<string> _list;
    _list.push_back("wendao");
    _list.push_front("tencent");
    _list.push_front("sina");
    _list.push_back("qihoo");

    list<string>::iterator iter;
    //sina  tencent wendao qihoo
    for(iter=_list.begin(); iter != _list.end(); iter++){
        cout<<*iter<<endl;
    }

    iter = _list.begin();
    iter++;
    iter++;
    _list.insert(iter,"baidu");
    //sina tencent baidu wendao qihoo
    for(iter=_list.begin(); iter!=_list.end(); iter++){
        cout<<*iter<<"\t";
    }
    cout<<endl;
    cout<<"list.size()="<<_list.size()<<endl;//5
    cout<<"list.front()="<<_list.front()<<endl;//sina
    cout<<"list.back()="<<_list.back()<<endl;//qihoo
    _list.insert(iter,"tencent");
    _list.sort();
    //baidu qihoo sina tencent tencent wendao
    for(iter=_list.begin(); iter!=_list.end(); iter++){
        cout<<*iter<<"\t";
    }
    cout<<endl;
    _list.unique();
    //baidu qihoo sina tencent wendao
    for(iter=_list.begin(); iter!=_list.end(); iter++){
        cout<<*iter<<"\t";
    }
    cout<<endl;

    return 0;
}
