#include<string>
#include<cstring>
#include<cstdlib>

const char* getStr(const int num,const char c){
    char* p = (char*)malloc(sizeof(char)*num);
    memset(p,c,num);
    return p;
}

const int* getInt(int i){
    const int *p = new int(i);
    return p;
}

const char* getPos(){
    const char *p = new char(65);
    return p;
}

char* getCharArr(const int num){
    char* arr = new char[num];
    memset(arr,'B',num);
    return arr;
}
