#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

int main(){
    char str[] = "ennwendao-liuyongshuai-tencenteeee";

    char *p = strtok(str,"en");
    printf("tok=%s\n",p); //w
    printf("origin=%s\n",str); //ennw

    p = strtok(NULL,"en");
    printf("tok=%s\n",p); //dao-liuyo
    printf("origin=%s\n",str); //ennw

    p = strtok(NULL,"en");
    printf("tok=%s\n",p); //gshuai-t
    printf("origin=%s\n",str); //ennw

    p = strtok(NULL,"en");
    printf("tok=%s\n",p); //c
    printf("origin=%s\n",str); //ennw

    p = strtok(NULL,"en");
    printf("tok=%s\n",p); //t
    printf("origin=%s\n",str); //ennw

    p = strtok(NULL,"en");
    printf("tok=%s\n",p); //(null)
    printf("origin=%s\n",str); //ennw

    p = strtok(NULL,"en");
    printf("tok=%s\n",p); //(null)
    printf("origin=%s\n",str); //ennw
    return 0;
}
