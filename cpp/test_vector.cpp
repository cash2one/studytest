#include<vector>
#include<string>
#include<iostream>
#include"test_vector.h"

using namespace std;

test_vector::test_vector(){

}
int test_vector::run(){
    //vector<string> _vec;
    _vec.push_back("wendao");
    _vec.push_back("liuyongshuai");
    _vec.push_back("baidu");
    _vec.push_back("tencent");
    _vec.push_back("qihoo");

    vector<string>::iterator iter;

    //wendao  liuyongshuai  baidu  tencent qihoo
    for(iter=_vec.begin(); iter != _vec.end(); iter++){
        cout<<*iter<<"\t";
    }
    cout<<endl;

    //liuyongshuai
    cout<<_vec[1]<<endl;

    iter = _vec.begin() + 2;
    _vec.insert(iter,"aaaaaaaa");

    //wendao liuyongshuai aaaaaaaa baidu  tencent qihoo
    for(vector<string>::size_type i=0; i<_vec.size(); i++){
        cout<<_vec[i]<<"\t";
    }
    cout<<endl;

    //wendao
    cout<<"vector.front()="<<_vec.front()<<endl;
    //qihoo
    cout<<"vector.back()="<<_vec.back()<<endl;
    //baidu: iter + 3
    iter = _vec.begin();
    cout<<iter[3]<<endl;
    return 0;
}
