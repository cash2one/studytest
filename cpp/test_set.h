#ifndef _TEST_SET_H_
#define _TEST_SET_H_
#include<set>
#include<string>

using namespace std;

class test_set{
public:
    test_set();
    int run();

protected:
    set<string> _set;
};

#endif
