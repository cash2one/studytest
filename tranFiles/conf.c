#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdint.h>

#include "conf.h"

//parse the config file to buffer
void parse_config(const char* conf_file, struct global_conf * ret_conf)
{
    //check arguments
    FILE *fp = fopen(conf_file,"r");
    if(NULL == fp)
    {
        perror(conf_file);
        exit(EXIT_FAILURE);
    }

    //read conf file and parse items
    char buf[1024] = {0};
    char cmd_key[100] = {0};
    char cmd_value[1024] = {0};
    char* ptr = NULL;
    int i;
    while(!feof(fp))
    {
        bzero(buf,1024);
        fgets(buf,1024,fp);
        if(strlen(buf) <= 0)
        {
            continue;
        }
        buf[strlen(buf) - 1] = '\0';
        //get key value
        if((ptr=strchr(buf,'=')) != NULL)
        {
            bzero(cmd_key,100);
            bzero(cmd_value,1024);
            strncpy(cmd_key,buf,ptr-buf);
            strcpy(cmd_value,ptr+1);
            if(strlen(cmd_value) <= 0){
                continue;
            }
            if(strcmp(cmd_key,"pthread_num") == 0)
            {
                ret_conf->pthread_num = atoi(cmd_value);
            }
            else if(strcmp(cmd_key,"monitor_dir") == 0)
            {
                strcpy(ret_conf->monitor_dir,cmd_value);
            }
            else if(strcmp(cmd_key,"rsync_dest") == 0)
            {
                strcpy(ret_conf->rsync_dest,cmd_value);
            }
            else if(strcmp(cmd_key,"cmd") == 0)
            {
                strcpy(ret_conf->cmd,cmd_value);
            }
            else if(strcmp(cmd_key,"log_file") == 0)
            {
                strcpy(ret_conf->log_file,cmd_value);
            }
            else if(strcmp(cmd_key,"check_exists") == 0)
            {
                strcpy(ret_conf->check_exists,cmd_value);
            }
        }
    }
    fclose(fp);
}

