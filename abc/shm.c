#include<sys/types.h>
#include<sys/stat.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>

int main(int argc, char* argv[]){
    int shmid;
    if(argc < 2){
        if((shmid = shmget(IPC_PRIVATE,4096,0777)) < 0){
            perror("shmget failed");
            exit(EXIT_FAILURE);
        }
    }
    else{
        shmid = atoi(argv[1]);
    }

    char* addr = shmat(shmid,0,0);
    if(-1 == addr){
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    char buf[] = "liuyongshuai";
    strcpy(addr,buf);
    printf("%s\n",addr);//liuyongshuai

    struct shmid_ds shm_st;
    shmctl(shmid,IPC_STAT,&shm_st);
    printf("shm_st.shm_segsz=%lu\n",shm_st.shm_segsz);//4096
    printf("shm_st.shm_lpid=%lu\n",shm_st.shm_lpid);//4412
    printf("shm_st.shm_cpid=%lu\n",shm_st.shm_cpid);//25967
    printf("shm_st.shm_nattch=%d\n",shm_st.shm_nattch);//1
    printf("shm_st.shm_atime=%lu\n",shm_st.shm_atime);//1393578612
    printf("shm_st.shm_ctime=%lu\n",shm_st.shm_ctime);//1393577596
    printf("shm_st.shm_dtime=%lu\n",shm_st.shm_dtime);//1393578583

    shmdt(addr);
    return 0;
}
