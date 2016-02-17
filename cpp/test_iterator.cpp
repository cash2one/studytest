#include<iterator>
#include<vector>
#include<string>
#include<algorithm>
#include<iostream>
#include<deque>

#include "test_iterator.h"

using namespace std;

int test_iterator::run(){
    vector<string> svec,tvec;
    std::deque<string> foo,bar;
    svec.push_back("wendao");
    svec.push_back("baidu");
    svec.push_back("tencent");
    svec.push_back("sina");
    svec.push_back("qihoo");
    svec.push_back("sohu");
    svec.push_back("alipay");

    copy(svec.begin(),svec.end(),back_inserter(tvec));

    vector<string>::iterator iter = tvec.begin();
    //wendao  baidu tencent sina qihoo sohu alipay
    for(;iter!=tvec.end();iter++){
        foo.push_back(*iter);
        cout<<*iter<<"\t";
    }
    cout<<endl;

    copy(foo.begin(),foo.end(),front_inserter(bar));

    deque<string>::iterator diter = bar.begin();
    //alipay sohu qihoo sina tencent baidu wendao
    for(;diter!=bar.end();diter++){
        cout<<*diter<<"\t";
    }
    cout<<endl;



    return 0;
}
