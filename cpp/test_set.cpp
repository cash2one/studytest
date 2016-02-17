#include<set>
#include<string>
#include<iostream>
#include "test_set.h"

using namespace std;

test_set::test_set(){

}
int test_set::run(){
    //set<string> _set;
    _set.insert("liuyongshuai");
    pair<set<string>::iterator,bool> ret= _set.insert("wendao");
    //1  wendao
    cout<<ret.second<<"\t"<<*ret.first<<endl;
    _set.insert("sogou");
    _set.insert("sogou");
    _set.insert(ret.first,"for_insert");

    set<string>::iterator iter;
    //for_insert  liuyongshuai sogou  wendao
    for(iter=_set.begin();iter != _set.end(); iter++){
        cout<<*iter<<"\t";
    }
    cout<<endl;

    iter = _set.find("sogou");
    cout<<*iter<<endl;//sogou
    _set.insert(iter,"tencent");
    //for_insert liuyongshuai sogou  tencent wendao
    for(iter=_set.begin();iter != _set.end(); iter++){
        cout<<*iter<<"\t";
    }
    cout<<endl;
    cout<<_set.size()<<endl;//5

    return 0;
}
