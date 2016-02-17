#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<ctype.h>

void hello_world();
void qsortString(char *strArr[],const int leftIndex,const int rightIndex);
void trim(char buf[],char ch);
int check_string(const char* str,char type);
