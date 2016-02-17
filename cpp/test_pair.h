#ifndef _TEST_PAIR_H_
#define _TEST_PAIR_H_
#include<utility>
#include<string>

using namespace std;

class test_pair{
public:
    test_pair();
    int run();

protected:
    pair<string,int> _pair;
};
#endif
