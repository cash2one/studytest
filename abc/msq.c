#include<stdio.h>
#include<stdlib.h>
#include<sys/msg.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/ipc.h>

struct msgbuf{
    long mbyte;
    char mbuf[20];
};

int main(int argc, char* argv[]){
    int msgid;
    if(argc > 1){
        msgid = atoi(argv[1]);
    }
    else{
        msgid = msgget(IPC_PRIVATE,0755|IPC_CREAT);
        if(-1 == msgid){
            fprintf(stderr,"create msgq failed");
            exit(EXIT_FAILURE);
        }
    }


    //send message
    struct msgbuf msg;
    msg.mbyte=5;
    strcpy(msg.mbuf,"liuyongshuai");
    if(msgsnd(msgid,&msg,20,0) != 0){
        fprintf(stderr,"send message failed\n");
        exit(EXIT_FAILURE);
    }

    //print stat
    struct msqid_ds stat_buf;
    if(msgctl(msgid,IPC_STAT,&stat_buf) != 0){
        fprintf(stderr,"msgctl failed\n");
        exit(EXIT_FAILURE);
    }
    printf("msqid_ds.msg_qnum=%u\n",stat_buf.msg_qnum);//1
    printf("msqid_ds.msg_qbytes=%u\n",stat_buf.msg_qbytes);//65536
    printf("msqid_ds.msg_cbytes=%u\n",stat_buf.msg_cbytes);//40
    printf("msqid_ds.msg_lspid=%d\n",stat_buf.msg_lspid);//31369

    //receive msg
    int msg_size = msgrcv(msgid,&msg,20,8,IPC_NOWAIT);
    printf("%d\n",msg_size); //-1
    msg_size = msgrcv(msgid,&msg,20,5,IPC_NOWAIT);
    //msg_size=20     msg=liuyongshuai
    printf("msg_size=%d\tmsg=%s\n",msg_size,msg.mbuf);

    return 0;
}
