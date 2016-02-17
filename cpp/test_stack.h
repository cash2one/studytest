#ifndef _TEST_STACK_H_
#define _TEST_STACK_H_
#include<stack>
#include<string>

using namespace std;

class test_stack{
public:
    test_stack();
    int run();

protected:
    stack<string> _stack;
};
#endif
