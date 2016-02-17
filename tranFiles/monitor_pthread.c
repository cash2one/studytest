#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<dirent.h>
#include<string.h>
#include<pthread.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<time.h>

#include "conf.h"
#include "utils.h"
#include "file_list.h"

//get file list,lock mutex,send to share memory
void* monitor_pthread(void* arg)
{
    //check dir access
    struct global_conf *conf = (struct global_conf*)arg;
    if(access(conf->monitor_dir,W_OK) != 0)
    {
        perror(conf->monitor_dir);
        exit(EXIT_FAILURE);
    }

    //strip last /
    if('/' == conf->monitor_dir[strlen(conf->monitor_dir) - 1])
    {
        conf->monitor_dir[strlen(conf->monitor_dir) - 1] = '\0';
    }

    //add files to link
    DIR *dp = NULL;
    struct dirent *dir_file = NULL;
    char filename[255] = {0};
    struct stat st;
    while(1)
    {
        sleep(2);
        if(conf->header->num > conf->pthread_num)
        {
            continue;
        }
        dp = opendir(conf->monitor_dir);
        pthread_mutex_lock(&conf->mutex);
        fprintf(stderr,"[monitor pthread_mutex_lock]monitor thread lock the mutex success\n");
        while((dir_file=readdir(dp)) != NULL)
        {
            if(strchr(dir_file->d_name,'.') == dir_file->d_name)
            {
                continue;
            }
            if(strchr(dir_file->d_name,'_') == dir_file->d_name)
            {
                continue;
            }
            //get full filename
            bzero(filename,255);
            sprintf(filename,"%s/%s",conf->monitor_dir,dir_file->d_name);
            if(stat(filename,&st) != 0)
            {
                continue;
            }
            //only care reg files
            if( !S_ISREG(st.st_mode))
            {
                continue;
            }
            add(conf->header,filename,conf->log_file);
        }
        pthread_mutex_unlock(&conf->mutex);
        fprintf(stderr,"[monitor pthread_mutex_unlock]monitor thread unlock the mutex\n");
        fprintf(stderr,"[monitor pthread_cond_broadcast]monitor thread start broadcast signal to cond\n");
        pthread_cond_broadcast(&conf->cond);
        closedir(dp);
    }

    return ((void*)0);
}
