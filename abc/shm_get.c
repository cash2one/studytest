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
        printf("invalid parameter\n");
        exit(EXIT_FAILURE);
    }
    else{
        shmid = atoi(argv[1]);
    }


    char* addr = shmat(shmid,0,0);
    if(-1 == addr){
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    char buf[] = "tencent-baidu-sina";
    strcpy(addr,buf);
    printf("%s\n",addr);

    struct shmid_ds shm_st;
    shmctl(shmid,IPC_STAT,&shm_st);
    printf("shm_st.shm_segsz=%lu\n",shm_st.shm_segsz);
    printf("shm_st.shm_lpid=%lu\n",shm_st.shm_lpid);
    printf("shm_st.shm_cpid=%lu\n",shm_st.shm_cpid);
    printf("shm_st.shm_nattch=%d\n",shm_st.shm_nattch);
    printf("shm_st.shm_atime=%lu\n",shm_st.shm_atime);
    printf("shm_st.shm_ctime=%lu\n",shm_st.shm_ctime);
    printf("shm_st.shm_dtime=%lu\n",shm_st.shm_dtime);


    shmdt(addr);
    return 0;
}
