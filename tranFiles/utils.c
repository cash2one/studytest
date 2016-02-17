#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdint.h>

//replace str
void str_replace(const char* str,char ret_buf[],uint32_t ret_size,const char* find,const char* replace)
{
    if(strlen(find) <= 0)
    {
        return;
    }
    if(strstr(str,find) == NULL)
    {
        return;
    }
    if(ret_size < strlen(str) - strlen(find) + strlen(replace) || strlen(str) <= 0)
    {
        return;
    }

    //start replace
    bzero(ret_buf,ret_size);
    char* p = str;
    char* tmp = NULL;
    while((tmp=strstr(p,find)) != NULL)
    {
        strncat(ret_buf,p,tmp-p);
        strcat(ret_buf,replace);
        p = tmp + strlen(find);
    }
    if(*p != '\0')
    {
        strcat(ret_buf,p);
    }
}

