#ifndef _TEST_MAP_H_
#define _TEST_MAP_H_
#include<string>
#include<map>

using namespace std;

class test_map{
public:
    test_map();
    int run();

protected:
    map<string,int> _map;
};
#endif
