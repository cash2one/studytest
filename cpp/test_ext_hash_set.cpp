#include<iostream>

#include "test_ext_hash_set.h"
#include "stdfx.h"

using namespace __gnu_cxx;
using namespace std;

int test_ext_hash_set::run(){
    hash_set<string> hs;

    hs.insert("wendao");
    hs.insert("liuyongshuai");
    hs.insert("tencent");
    hs.insert("wendao");

    hash_set<string>::iterator iter;
    //wendao tencent liuyongshuai
    for(iter=hs.begin();iter!=hs.end();iter++){
        cout<<*iter<<"\t";
    }
    cout<<endl;

    cout<<"hs.size()="<<hs.size()<<endl;
    cout<<"hs.max_size()="<<hs.max_size()<<endl;
    cout<<"hs.empty()="<<hs.empty()<<endl;

    iter = hs.find("wendao");
    cout<<*iter<<endl;

    return 0;
}
