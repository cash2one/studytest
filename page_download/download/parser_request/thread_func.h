#include<stdio.h>
#include<stdlib.h>
#include<string>

using namespace std;

int32_t start(const string& seed, const string& config);
void* parser_func(void* n);
void* dispatch_func(void *);
void do_global_counter(int num);
int get_engine_list();
