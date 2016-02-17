#include<ext/slist>
#include<iostream>
#include<string>

#include "test_ext_hash_slist.h"

using namespace std;
using namespace __gnu_cxx;

int test_ext_hash_slist::run(){
    slist<string> st;

    st.push_front("wendao");
    st.push_front("liuyongshuai");
    st.push_front("tencent");
    st.push_front("sina");
    st.push_front("sogou");
    st.push_front("baidu");

    slist<string>::iterator iter;
    for(iter=st.begin();iter!=st.end();iter++){
        cout<<*iter<<"\t";
    }
    cout<<endl;

    st.insert(iter,"qihoo");
    st.insert_after(st.begin(),"sohu");
    for(iter=st.begin();iter!=st.end();iter++){
        cout<<*iter<<"\t";
    }
    cout<<endl;

    st.sort();
    for(iter=st.begin();iter!=st.end();iter++){
        cout<<*iter<<"\t";
    }
    cout<<endl;
    return 0;
}
