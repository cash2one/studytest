#ifndef _TEST_CLASS_CHILD_H_
#define _TEST_CLASS_CHILD_H_
#include<string>
#include "test_class.h"

using namespace std;

class test_class_child:public test_class{
public:
    test_class_child(int flag, int i):test_class(flag),num1(i){

    }
    int num1;
    void printStr();
    void printTest();
};

#endif
