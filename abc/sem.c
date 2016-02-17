#include<sys/types.h>
#include<sys/stat.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>

union  semun{
    int val;
    struct semid_ds *buf;
    unsigned  short  *array;
};

int main(int argc, char* argv[]){
    int semid;
    if(argc < 2){
        if((semid = semget(IPC_PRIVATE,3,0777)) < 0){
            perror("semget failed");
            exit(EXIT_FAILURE);
        }
    }
    else{
        semid = atoi(argv[1]);
    }

    union semun arg;
    arg.val = 1;
    semctl(semid,0,SETVAL,arg);
    arg.val = 1;
    semctl(semid,1,SETVAL,arg);
    arg.val = 1;
    semctl(semid,2,SETVAL,arg);

    printf("semval=%d\n",semctl(semid,0,GETVAL));
    printf("semval=%d\n",semctl(semid,1,GETVAL));
    printf("semval=%d\n",semctl(semid,2,GETVAL));

    struct sembuf op[3];
    op[0].sem_num = 0;
    op[0].sem_op = -1;
    op[0].sem_flg =  IPC_NOWAIT;
    op[1].sem_num = 1;
    op[1].sem_op = -1;
    op[2].sem_num = 1;
    op[2].sem_op = -1;
    if(semop(semid,op,3) != 0){
        perror("semop failed");
        exit(EXIT_FAILURE);
    }

    printf("semval=%d\n",semctl(semid,0,GETVAL));
    printf("semval=%d\n",semctl(semid,1,GETVAL));
    printf("semval=%d\n",semctl(semid,2,GETVAL));
    return 0;
}
