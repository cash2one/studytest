#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include "cmd_uniq.h"

/*
struct uniqLink{
    int num;
    char* str;
    struct uniqLink *next;
};
*/
static struct uniqLink *header=NULL,*last=NULL;
static void strLinkUniq(struct uniqLink *header);

//操作文件，显示出非重复的内容及重复的内容
int cmd_uniq(int argc, char *argv[]){
    if(argc < 3){
        fprintf(stderr,"wrong number of parameters\n");
        exit(EXIT_FAILURE);
    }

    //校验文件合法性
    const char* filename = argv[2];
    if(access(filename,R_OK) != 0){
        char errorBuf[255];
        sprintf(errorBuf,"read file failed %s ", filename);
        perror(errorBuf);
        exit(EXIT_FAILURE);
    }

    //读取文件内容
    FILE *fp = fopen(filename,"r");
    if(NULL == fp){
        perror("read file failed");
        exit(EXIT_FAILURE);
    }
    header = (struct uniqLink*)malloc(sizeof(struct uniqLink));
    header->num = 0;
    header->str=NULL;
    header->next=NULL;
    last = header;
    char buf[4096];
    while(!feof(fp)){
        bzero(buf,4096);
        fgets(buf,4096,fp);
        if(strlen(buf) <= 0){
            break;
        }
        struct uniqLink *tmp = (struct uniqLink*)malloc(sizeof(struct uniqLink));
        tmp->num=1;
        tmp->next=NULL;
        tmp->str = (char*)malloc(sizeof(char)*(strlen(buf)+1));
        strcpy(tmp->str,buf);
        last->next=tmp;
        header->num++;
        last=tmp;
    }
    fclose(fp);
    strLinkUniq(header);

    //打印输出
    last=header->next;
    while(NULL != last){
        printf("%d\t%s\n",last->num,last->str);
        last=last->next;
    }

    return 0;
}

static void strLinkUniq(struct uniqLink *header){
    struct uniqLink *cur=header->next,*post=NULL,*pre=NULL;
    while(NULL != cur){
        post = cur->next;
        pre = cur;
        while(NULL != post){
            if(strcmp(cur->str,post->str) == 0){
                cur->num++;
                pre->next = post->next;
                free(post->str);
                free(post);
            }
            pre = post;
            post=post->next;
        }
        cur = cur->next;
    }
}
