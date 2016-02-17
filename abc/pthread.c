#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>

struct abc{
    char buf[50];
    int a;
};
void clean_func(void *arg)
{
    printf("clean Msg:%s\n",(char*)arg);
}
void *thread1(void *a)
{
    struct abc *b = (struct abc*)a;
    pthread_t ptid = pthread_self();
    printf("thread1\tptid=%lu\tb.buf=%s\n",ptid,b->buf);
    return ((void *)0);
}
void *thread2(void *arg)
{
    printf("thread2---pthid=%lu\n",pthread_self());
    pthread_cleanup_push(clean_func,(void *)"cleanup func from thread2");
    pthread_exit("exit from thread2");
    pthread_cleanup_pop(0);
    return ((void *)0);
}
int main(int argc, char* argv[])
{
    pthread_t pthid1,pthid2;
    struct abc m;
    strcpy(m.buf,"wendao---");
    m.a = 40;

    if(pthread_create(&pthid1,NULL,thread1,&m) != 0)
    {
        perror("create pthread failed");
        exit(EXIT_FAILURE);
    }
    if(pthread_create(&pthid2,NULL,thread2,NULL) != 0)
    {
        perror("create pthread failed");
        exit(EXIT_FAILURE);
    }
    printf("main pthread pthid1=%lu\tpthid2=%lu\n",pthid1,pthid2);

    void *thread2_ret;
    pthread_join(pthid1,NULL);
    pthread_join(pthid2,&thread2_ret);
    printf("thread2_ret:%s\n",(char *)thread2_ret);

    return 0;
}



