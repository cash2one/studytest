#include<stdio.h>
#include<stdint.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#include "utils.h"
#include "conf.h"
#include "send_pthread.h"
#include "monitor_pthread.h"


int main(int argc, char* argv[])
{
    struct global_conf conf = {0};
    conf.header = (struct file_link_header *)malloc(sizeof(struct file_link_header));
    conf.header->next = NULL;
    conf.header->num = 0;
    pthread_mutex_init(&conf.mutex,NULL);
    pthread_cond_init(&conf.cond,NULL);

    //parse config file
    parse_config("trans.conf",&conf);
    pthread_t ptid[conf.pthread_num];

    //create monitor pthread
    pthread_t monitor_ptid;
    if(pthread_create(&monitor_ptid,NULL,monitor_pthread,(void*)&conf) != 0)
    {
        perror("create monitor thread failed");
        exit(EXIT_FAILURE);
    }


    //create send file pthread
    int i;
    for(i=0; i<conf.pthread_num; i++)
    {
        if(pthread_create(&ptid[i],NULL,send_pthread,(void*)&conf) != 0)
        {
            perror("create send thread failed");
            exit(EXIT_FAILURE);
        }
    }

    //pthread_join all pthreads
    for(i=0; i<conf.pthread_num; i++)
    {
        pthread_join(ptid[i],NULL);
    }
    pthread_join(monitor_ptid,NULL);

    //release space
    pthread_mutex_destroy(&conf.mutex);
    pthread_cond_destroy(&conf.cond);
    free(conf.header);

    printf("main thread finished\n");
    return 0;
}
