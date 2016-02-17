#ifndef _TEST_QUEUE_H_
#define _TEST_QUEUE_H_
#include<queue>
#include<string>

using namespace std;

class test_queue{
public:
    test_queue();
    int run();

protected:
    queue<string> _queue;
};
#endif
