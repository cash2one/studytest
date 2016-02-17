#ifndef _TEST_STRING_H_
#define _TEST_STRING_H_
#include<string>

class test_string{
public:
    test_string(char *str);
    test_string(std::string str);
    int run();

protected:
    std::string test_str;
};
#endif
