#ifndef _TEST_LIST_H_
#define _TEST_LIST_H_H
#include<list>
#include<string>

using namespace std;

class test_list{
public:
    test_list();
    int run();

protected:
    list<string> _list;
};
#endif
