#include<ext/hash_map>
#include<iostream>
#include<string>
#include "test_ext_hash_map.h"
#include "stdfx.h"

using namespace std;
using namespace __gnu_cxx;

int test_ext_hash_map::run(){
    hash_map<string,string> hm;

    hm.insert(make_pair("first","wendao"));
    hm.insert(make_pair("second","liuyongshuai"));
    hm.insert(pair<string,string>("third","tencent"));
    hm["four"] = "sina";

    hash_map<string,string>::iterator iter;
    for(iter=hm.begin();iter!=hm.end();iter++){
        cout<<iter->first<<"\t"<<iter->second<<endl;
    }

    cout<<"hm.size()="<<hm.size()<<endl;
    cout<<"hm.max_size()="<<hm.max_size()<<endl;
    cout<<"hm.empty()="<<hm.empty()<<endl;

    iter = hm.find("second");
    cout<<iter->first<<"\t"<<iter->second<<endl;

    return 0;
}
