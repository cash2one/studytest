#include<iostream>
#include<string>
#include<cstdio>
#include<vector>
#include<algorithm>

using namespace std;

struct test{
    int a;
    int b;
    char buf[255];
};
//for qsort
bool compare(struct test* a, struct test* b)
{
    return (a->a > b->a);
}

int main(){
    vector<struct test*> vec;

    struct test* m1 = (struct test*)malloc(sizeof(struct test));
    m1->a=4;
    m1->b=9;
    vec.push_back(m1);

    m1 = (struct test*)malloc(sizeof(struct test));
    m1->a=6;
    m1->b=8;
    vec.push_back(m1);

    m1 = (struct test*)malloc(sizeof(struct test));
    m1->a=2;
    m1->b=11;
    vec.push_back(m1);

    m1 = (struct test*)malloc(sizeof(struct test));
    m1->a=67;
    m1->b=45;
    vec.push_back(m1);

    sort(vec.begin(), vec.end(), compare);

    vector<struct test*>::iterator iter;
    for(iter=vec.begin(); iter!=vec.end(); iter++){
        cout<<(*iter)->a<<"\t"<<(*iter)->b<<endl;
    }

    return 0;
}
