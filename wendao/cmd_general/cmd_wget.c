#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

int cmd_wget(int argc, char* argv[]){
    if(argc < 4){
        fprintf(stderr,"wrong number of parameter\n");
        exit(EXIT_FAILURE);
    }

    //获取参数
    char urlBuf[1024] = {0};
    char localBuf[255] = {0};
    strcpy(urlBuf,argv[2]);


    strcpy(localBuf,argv[3]);
    FILE *fp = fopen(localBuf,"w+");
    if(NULL == fp){
        perror(localBuf);
        exit(EXIT_FAILURE);
    }


    fprintf(stderr,"cmd_wget..........\n");
    return 0;
}
