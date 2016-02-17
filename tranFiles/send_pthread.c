#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/mman.h>
#include<pthread.h>
#include<fcntl.h>
#include<unistd.h>

#include "conf.h"
#include "utils.h"
#include "file_list.h"

static struct global_conf *conf = NULL;

static int send_file(const char* log_file, const char* file);

//the pthread of send
//lock mutex,get file for sending from share memory
void* send_pthread(void *arg)
{
    conf = (struct global_conf*)arg;
    char file_name[255] = {0};
    while(1)
    {
        bzero(file_name,255);
        fprintf(stderr,"[send pthread_mutex_lock]thread %lu start to lock mutex\n",pthread_self());
        pthread_mutex_lock(&conf->mutex);
        fprintf(stderr,"[send pthread_mutex_lock]thread %lu lock mutex success\n",pthread_self());
        if(conf->header->num > 0)
        {
            delete(conf->header,file_name,255);
        }
        else
        {
            fprintf(stderr,"[send pthread_cond_wait]thread %lu start to wait for new file\n",pthread_self());
            pthread_cond_wait(&conf->cond,&conf->mutex);
        }
        pthread_mutex_unlock(&conf->mutex);
        fprintf(stderr,"[send pthread_mutex_unlock]thread %lu unlock mutex\n",pthread_self());
        if(strlen(file_name) > 0)
        {
            send_file(conf->log_file,file_name);
        }
    }
    return ((void*)0);
}


//send files and delete local files
static int send_file(const char* log_file, const char* file)
{
    char cmd_buf[4096] = {0};
    char cmd_buf_tmp[4096] = {0};
    str_replace(conf->cmd,cmd_buf,4096,"%{file}",file);
    str_replace(cmd_buf,cmd_buf_tmp,4096,"%{rsync_dest}",conf->rsync_dest);
    fprintf(stderr,"[start send file]%s\n",cmd_buf_tmp);
    system(cmd_buf_tmp);

    bzero(cmd_buf,4096);
    bzero(cmd_buf_tmp,4096);
    str_replace(conf->check_exists,cmd_buf,4096,"%{rsync_dest}",conf->rsync_dest);
    str_replace(cmd_buf,cmd_buf_tmp,4096,"%{file}",strrchr(file,'/') + 1);
    fprintf(stderr,"[check send success]%s\n",cmd_buf_tmp);
    FILE *fp = popen(cmd_buf_tmp,"r");
    bzero(cmd_buf,4096);
    fgets(cmd_buf,4096,fp);
    if(strlen(cmd_buf) > 0)
    {
        //write log
        int fd = open(log_file,O_WRONLY|O_APPEND|O_CREAT,0755);
        char file_buf[1024] = {0};
        sprintf(file_buf,"%s\n",file);
        write(fd,file_buf,strlen(file_buf));
        close(fd);
        //delete file
        fprintf(stderr,"[confirm send success]remove %s........\n",file);
        remove(file);
    }


    return 0;
}
