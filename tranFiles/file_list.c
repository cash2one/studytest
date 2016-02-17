#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<string.h>

#include "conf.h"

//check file exists log
static uint32_t check_file_send(const char* log_file,const char* filename)
{
    if(access(log_file,R_OK) != 0)
    {
        return 0;
    }
    FILE *fp = fopen(log_file,"r");
    if(NULL == fp)
    {
        return 0;
    }
    char buf[255];
    while(!feof(fp))
    {
        bzero(buf,255);
        fgets(buf,255,fp);
        if(strlen(buf) <= 1)
        {
            continue;
        }
        if(buf[strlen(buf)-1] == '\n')
        {
            buf[strlen(buf)-1] = '\0';
        }
        if(strcmp(buf,filename) == 0)
        {
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

//filename:/search/test/abc.txt
void add(struct file_link_header *header,const char* filename,const char* log_file)
{
    //get file stat
    struct stat st;
    if(stat(filename,&st))
    {
        fprintf(stderr,"get file stat failed:%s\n",filename);
    }

    if(check_file_send(log_file,filename) != 0)
    {
        return;
    }

    //check filename
    struct file_link *cur = header->next;
    struct file_link *pre = NULL;
    while(cur != NULL)
    {
        if(strcmp(cur->name,filename) == 0)
        {
            return;
        }
        cur = cur->next;
    }

    //create node
    struct file_link *tmp = (struct file_link*)malloc(sizeof(struct file_link));
    bzero(tmp,sizeof(struct file_link));
    strcpy(tmp->name,filename);
    tmp->modify_time = st.st_mtime;
    tmp->next = NULL;

    //insert into list
    cur = header->next,pre = NULL;
    while(cur != NULL && cur->modify_time < tmp->modify_time)
    {
        pre = cur;
        cur = cur->next;
    }
    if(NULL == pre)
    {
        tmp->next = header->next;
        header->next = tmp;
    }
    else
    {
        tmp->next = pre->next;
        pre->next = tmp;
    }
    header->num++;

    //debug info
    fprintf(stderr,"[monitor add]add %s to list for send,header->num=%d\ttime=%lu\n",filename,header->num,tmp->modify_time);
}

//get one file and delete from file_link
void delete(struct file_link_header *header,char file_buf[], uint32_t buf_size)
{
    if(header->num > 0 && header->next == NULL)
    {
        fprintf(stderr,">>>>>>>>>>>>>>>>abnormal cond>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        fprintf(stderr,"header->num=%d\n",header->num);
        exit(EXIT_FAILURE);
    }
    if(header->num > 0 && header->next != NULL)
    {
        struct file_link *p = header->next->next;
        if(strlen(header->next->name) > buf_size)
        {
            return;
        }
        strcpy(file_buf,header->next->name);
        free(header->next);
        header->next = p;
        header->num--;

        //debug info
        p = header->next;
        fprintf(stderr,"++++++++++++++header->num=%d++++++++++++++++++\n",header->num);
        while(p!=NULL)
        {
            fprintf(stderr,"%s\t%lu\n",p->name,p->modify_time);
            p = p->next;
        }
    }
}

