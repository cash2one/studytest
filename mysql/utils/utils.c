#include "utils.h"

void hello_world(){
    fprintf(stderr,"hello world\n");
}
//快速排序字符串数组
void qsortString(char *strArr[],const int leftIndex,const int rightIndex){
    if(leftIndex >= rightIndex){
        return;
    }
    int start = leftIndex, end=rightIndex;
    char *midStr = strArr[leftIndex];
    while(start < end){
        while(start < end && strcmp(strArr[end],midStr) >= 0){
            end--;
        }
        if(start < end){
           strArr[start++] = strArr[end];
        }
        while(start < end && strcmp(strArr[start],midStr) < 0){
            start++;
        }
        if(start < end){
            strArr[end--]=strArr[start];
        }
    }
    strArr[start]=midStr;
    qsortString(strArr,leftIndex,start-1);
    qsortString(strArr,start+1,rightIndex);
}

//去掉字符串两边的特定字符
void trim(char buf[],char ch){
    if(strlen(buf) <= 0){
        return;
    }
    char *str=buf;
    while(*str == ch){
         str += 1;
    }
    int start = 0,diff_len = str - buf;
    int length = strlen(buf) - diff_len;
    for(; start < length; start++){
        buf[start] = buf[start+diff_len];
    }
    length = strlen(str);
    while(*(str+length-1) == ch){
        *(str+length-1)=0;
        length = strlen(str);
    }
}
//检查字符串的内容是否全是某类型字符，如数字、字母
//type = 1：检查此字符串是否全为数字
//type = 2：检查此字符串是否全为字母
//type = 3：检查此字符串是否是由数字、字母组成
int check_string(const char* str,char type){
    if(strlen(str) <= 0){
        return 1;
    }
    int i;
    for(i=0; i<strlen(str); i++){
        switch(type){
            case 1:
                if(!isdigit(*(str+i))){
                    return 1;
                }
                break;
            case 2:
                if(!isalpha(*(str+i))){
                    return 1;
                }
                break;
            case 3:
                if(!isalnum(*(str+i))){
                    return 1;
                }
                break;
            default:
                return 1;
        }
    }
    return 0;
}
