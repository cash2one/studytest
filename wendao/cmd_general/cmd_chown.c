#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<stdlib.h>

//更改文件所有者及组
//owner[.group] 用户名或者组名
//file 文件名
int cmd_chown(int argc, char* argv[]){
    if(argc < 4){
        fprintf(stderr,"wrong number of parameter\n");
        exit(EXIT_FAILURE);
    }

    //接收参数
    char owner[50] = {0};
    char group[50] = {0};
    if(strchr(argv[2],'.') != NULL){
        char *p = strtok(argv[2],".");
        strcpy(owner,p);
        strcpy(group,strtok(NULL,"."));
    }
    else{
        strcpy(owner,argv[2]);
    }

    char fileBuf[255] = {0};
    strcpy(fileBuf,argv[3]);
    if(access(fileBuf,W_OK) != 0){
        perror(fileBuf);
        exit(EXIT_FAILURE);
    }

    uid_t uid[1] = {0};
    gid_t gid[1] = {0};
    uname2uid(uid,gid,owner);

    chown(fileBuf,uid[0],gid[0]);

    return 0;
}
