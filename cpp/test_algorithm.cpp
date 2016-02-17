#include<iostream>
#include<string>
#include<vector>
#include<set>
#include<map>
#include<list>
#include<algorithm>
#include<numeric>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#include "test_algorithm.h"

using namespace std;
using namespace __gnu_cxx;

struct abc{
    int a;
    int b;
};

struct abc* accumul(struct abc* a, const struct abc* b){
    a->a += b->a;
    a->b += b->b;
    return a;
}
bool count_if_func(const struct abc* a){
    return ((a->a)%2==0)?true:false;
}
void for_each_func(const struct abc* a){
    printf("a->a=%d\ta->b=%d\n",a->a,a->b);
}
bool max_element_func(const struct abc* a,const struct abc* b){
    return a->a < b->a;
}

int test_algorithm::run()
{
    vector<struct abc*> vec;
    vector<struct abc*>::iterator iter = vec.begin();
    struct abc* ptr = NULL;
    int sum_a=0, sum_b = 0;
    for(int i=0; i < 10; i++){
        ptr = (struct abc*)malloc(sizeof(struct abc));
        ptr->a = rand() % 100;
        ptr->b = rand() % 10;
        vec.push_back(ptr);
        sum_a += ptr->a;
        sum_b += ptr->b;
        printf("a=%d\tb=%d\n",ptr->a,ptr->b);
    }
    printf("sum_a=%d\tsum_b=%d\n",sum_a,sum_b);
    ptr = (struct abc*)malloc(sizeof(struct abc));
    ptr->a = rand() % 100;
    ptr->b = rand() % 10;
    cout<<"last ptr->a="<<ptr->a<<"\tptr->b="<<ptr->b<<endl;

    cout<<"-----------accumulate--------------"<<endl;
    ptr = accumulate(vec.begin(), vec.end(), ptr, accumul);
    cout<<ptr->a<<endl;
    for(int i=0; i < 10; i++){
        cout<<"vec["<<i<<"]->a="<<vec[i]->a<<"\tvec["<<i<<"]->b="<<vec[i]->b<<endl;
    }
    cout<<ptr->a<<endl;

    cout<<"-----------copy--------------"<<endl;
    vector<struct abc*> tmp(4);
    iter = vec.begin();
    copy(iter, iter+4, tmp.begin());
    for(iter=tmp.begin();iter!=tmp.end();iter++){
        cout<<"(*iter)->a="<<(*iter)->a<<"\t";
    }
    cout<<endl;

    cout<<"-----------copy_n--------------"<<endl;
    iter = vec.begin();
    copy_n(iter,4,tmp.begin());
    for(iter=tmp.begin();iter!=tmp.end();iter++){
        cout<<"(*iter)->a="<<(*iter)->a<<"\t";
    }
    cout<<endl;

    cout<<"-----------count_if--------------"<<endl;
    cout<<count_if(vec.begin(),vec.end(),count_if_func)<<endl;

    cout<<"-----------find_if---------------"<<endl;
    iter = find_if(vec.begin(),vec.end(),count_if_func);
    cout<<(*iter)->a<<endl;

    cout<<"-----------for_each---------------"<<endl;
    for_each(vec.begin(),vec.end(),for_each_func);

    cout<<"-----------max_element-------------"<<endl;
    iter = max_element(vec.begin(),vec.end(),max_element_func);
    cout<<(*iter)->a<<endl;

    cout<<"-------------sort------------------"<<endl;
    sort(vec.begin(),vec.end(),max_element_func);
    for(iter=vec.begin();iter!=vec.end();iter++){
        cout<<"(*iter)->a="<<(*iter)->a<<endl;
    }

    return 0;
}
