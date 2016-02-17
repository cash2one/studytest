#ifndef _TEST_VECTOR_H_
#define _TEST_VECTOR_H_
#include<vector>
#include<string>

using namespace std;

class test_vector{
public:
    test_vector();
    int run();

protected:
    vector<string> _vec;
};
#endif
