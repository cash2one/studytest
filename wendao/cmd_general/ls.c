#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<time.h>
#include "ls.h"

#define L_FLAG 0b00000001
#define T_FLAG 0b00000010
#define R_FLAG 0b00000100
#define A_FLAG 0b00001000
#define D_FLAG 0b00010000
#define M_FLAG 0b00100000
#define S_FLAG 0b01000000

static struct ls_finfo *header=NULL;
static struct ls_finfo *last=NULL;

static char option = 0b00000000;

//wendao ls <work-dir> <option>
//work-dir:Ҫ������Ŀ¼��λ�ÿ�����
//option:����ѡ������Ž������ʾ����ֵ���£�
//-l:һ��һ������ʾȨ�ޡ������ߡ���С��ʱ�䡢�ļ�������Ϣ
//-t:������޸�ʱ������
//-r:��������
//-a:��ʾ�����ļ������������ļ�
//-d:ֻ��ʾĿ¼
//-m:�Զ��Ÿ�����ʾ��һ��
//-s:���ļ���С������ʾ
int cmd_ls(int argc, char *argv[]){
    //Ѱ�ҵ�һ������"-"��ͷ�Ĳ�����ΪҪ������Ŀ¼���Ҳ�����Ϊ��ǰĿ¼
    char dir[256] = {0};
    int ind=2;
    for(; ind<argc; ind++){
        if(strchr(argv[ind],'-') == argv[ind]){
            continue;
        }
        else{
            strcpy(dir,argv[ind]);
            break;
        }
    }
    if(strlen(dir) <= 0){
        getcwd(dir,256);
    }
    if(access(dir,R_OK) != 0){
        printf("read dir %s failed\n",dir);
        exit(EXIT_FAILURE);
    }
    if(dir[sizeof(dir)-1]=='/'){
        dir[sizeof(dir)-1]='\0';
    }

    //��ȡ���������ø�����־λ
    int ch;
    while((ch=getopt(argc,argv,"ltradms")) != -1){
        switch(ch){
            case 'l':option += L_FLAG;break;
            case 't':option += T_FLAG;break;
            case 'r':option += R_FLAG;break;
            case 'a':option += A_FLAG;break;
            case 'd':option += D_FLAG;break;
            case 'm':option += M_FLAG;break;
            case 's':option += S_FLAG;break;
            default:break;
        }
    }

    //��ȡҪ����Ŀ¼������
    DIR *dp = opendir(dir);
    if(NULL == dp){
        perror("open dir failed");
        exit(EXIT_FAILURE);
    }
    struct dirent *dirItem;
    header=(struct ls_finfo*)malloc(sizeof(struct ls_finfo));
    last = header;
    while((dirItem=readdir(dp)) != NULL){
        if(!(option & A_FLAG)){
            if(*dirItem->d_name=='.'){
                continue;
            }
        }


        //��ȡ�ļ��ĸ���������Ϣ
        char filename[255];
        sprintf(filename,"%s/%s",dir,dirItem->d_name);
        struct stat fst;
        stat(filename,&fst);
        if((option & D_FLAG) && !S_ISDIR(fst.st_mode)){
            continue;
        }

        struct ls_finfo *tmp=(struct ls_finfo*)malloc(sizeof(struct ls_finfo));
        tmp->next = NULL;
        tmp->mtime=fst.st_mtime;
        parseStatMode(tmp->modeBuf,11,fst.st_mode);
        uid2name(tmp->uname,20,fst.st_uid,tmp->gname,20);
        strcpy(tmp->fname,dirItem->d_name);
        tmp->size=fst.st_size;
        tmp->nlink=fst.st_nlink;

        last->next = tmp;
        last = tmp;
        header->size++;
    }
    sortList();
    last = header->next;
    printf("%d items total\n",header->size);
    while(last != NULL){
        if(option == 0){
            printf("%s\t",last->fname);
        }
        else if(option & M_FLAG){
            printf("%s,",last->fname);
        }
        else{
            struct tm *tp=gmtime(&last->mtime);
            char timeBuf[20];
            sprintf(timeBuf,"%04d-%02d-%02d %02d:%02d:%02d",tp->tm_year+1900,tp->tm_mon+1,tp->tm_mday,tp->tm_hour,tp->tm_min,tp->tm_sec);
            printf("%s %d\t%s\t%s\t%d\t%s\t%s\n",last->modeBuf,last->nlink,last->uname,last->gname,last->size,timeBuf,last->fname);
        }
        last=last->next;
    }
}

static void swapFinfo(struct ls_finfo* const cur, struct ls_finfo* const post){
    int tmpInt=0;
    tmpInt=cur->nlink;
    cur->nlink=post->nlink;
    post->nlink=tmpInt;

    long tmpLong=0L;
    tmpLong = cur->mtime;
    cur->mtime = post->mtime;
    post->mtime = tmpLong;

    tmpLong = cur->size;
    cur->size = post->size;
    post->size = tmpLong;

    char tmpBuf[256];
    strcpy(tmpBuf,cur->modeBuf);
    strcpy(cur->modeBuf,post->modeBuf);
    strcpy(post->modeBuf,tmpBuf);

    bzero(tmpBuf,256);
    strcpy(tmpBuf,cur->uname);
    strcpy(cur->uname,post->uname);
    strcpy(post->uname,tmpBuf);

    bzero(tmpBuf,256);
    strcpy(tmpBuf,cur->gname);
    strcpy(cur->gname,post->gname);
    strcpy(post->gname,tmpBuf);

    bzero(tmpBuf,256);
    strcpy(tmpBuf,cur->fname);
    strcpy(cur->fname,post->fname);
    strcpy(post->fname,tmpBuf);
}

//t��ʱ������s����С����
int sortList(){
    struct ls_finfo *cur=header->next,*post=NULL;
    while(cur != NULL){
        post = cur->next;
        while(post != NULL){
            if((option & S_FLAG) && !(option & R_FLAG)){
                if(cur->size > post->size){
                    swapFinfo(cur,post);
                }
            }
            else if((option & S_FLAG) && (option & R_FLAG)){
                if(cur->size < post->size){
                    swapFinfo(cur,post);
                }
            }
            else if((option & T_FLAG) && !(option & R_FLAG)){
                if(cur->mtime > post->mtime){
                    swapFinfo(cur,post);
                }
            }
            else if((option & T_FLAG) && (option & R_FLAG)){
                if(cur->mtime < post->mtime){
                    swapFinfo(cur,post);
                }
            }
            else{
                if(strcmp(cur->fname,post->fname)>0){
                    swapFinfo(cur,post);
                }
            }
            post = post->next;
        }
        cur = cur->next;
    }

}
