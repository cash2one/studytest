#ifndef _TEST_CLASS_H_
#define _TEST_CLASS_H_

#include<iostream>
#include<string>

using namespace std;

class test_class{
public:
    int num;
    static int i;

    test_class(); //default constructor
    test_class(int flag):num(flag){
        cout<<"with one parameter,flag="<<flag<<endl;
    }
    test_class(const test_class &inst);
    test_class& operator=(const test_class &inst);
    test_class& operator+=(const test_class &inst);
    operator int();
    ~test_class();

    static void show();
    void showAll() const;
    virtual void printStr();
    void printTest();
};
#endif
