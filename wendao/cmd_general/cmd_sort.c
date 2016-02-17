#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include "cmd_sort.h"

char order = 'n';

/*
struct strLink{
    char *str;
    struct strLink *next;
};
*/
static void strLinkSort(struct strLink* strlink);
static struct strLink *header;
static struct strLink *last;

//接受一个文件作为参数
//-r：逆序
int cmd_sort(int argc, char *argv[]){
    if(argc < 3 ){
        fprintf(stderr,"wrong number of parameters\n");
        exit(EXIT_FAILURE);
    }

    //检测文件是否可读
    const char* filename = argv[2];
    if(access(filename,R_OK) != 0){
        char errorBuf[255];
        sprintf(errorBuf, "read file failed:%s ", filename);
        perror(errorBuf);
        exit(EXIT_FAILURE);
    }


    //解析参数
    int ch;
    while((ch=getopt(argc,argv,"r")) != -1){
        switch(ch){
            case 'r':order = 'r';break;
            default:break;
        }
    }

    //读取文件内容
    FILE *fp = fopen(filename,"r");
    if(NULL == fp){
        perror("fopen file failed");
        exit(EXIT_FAILURE);
    }
    char buf[4096];
    header = (struct strLink*)malloc(sizeof(struct strLink));
    header->str=NULL;
    header->next=NULL;
    last = header;
    while(!feof(fp)){
        bzero(buf,4096);
        fgets(buf,4096,fp);
        if(strlen(buf) <= 0){
            break;
        }
        struct strLink *link_tmp = (struct strLink *)malloc(sizeof(struct strLink));
        link_tmp->str = (char *)malloc(sizeof(char) * (strlen(buf)+1));
        strcpy(link_tmp->str,buf);
        link_tmp->next = NULL;
        last->next = link_tmp;
        last = link_tmp;
    }
    fclose(fp);
    strLinkSort(header);
    last = header->next;
    while(NULL != last){
        printf("%s",last->str);
        last = last->next;
    }

    return 0;
}

static void strLinkSort(struct strLink* strlink){
    struct strLink *cur = strlink->next;
    struct strLink *post=NULL;
    char* tmp;
    while(cur != NULL){
        post = cur->next;
        while(NULL != post){
            if('n' == order){
                if(strcmp(cur->str,post->str) > 0){
                    tmp = cur->str;
                    cur->str = post->str;
                    post->str = tmp;
                }
            }
            else if('r' == order){
                if(strcmp(cur->str,post->str) < 0){
                    tmp = cur->str;
                    cur->str = post->str;
                    post->str = tmp;
                }
            }
            post = post->next;
        }
        cur = cur->next;
    }
}
