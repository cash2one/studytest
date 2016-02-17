#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdint.h>

int cmd_chmod(int argc, char* argv[]){
    if(argc < 4){
        fprintf(stderr,"wrong number of parameter\n");
        exit(EXIT_FAILURE);
    }

    if(check_string(argv[2],1) != 0 || *(argv[2]) != '0'){
        fprintf(stderr,"invalid mode\n");
        exit(EXIT_FAILURE);
    }
    mode_t mt = strtoul(argv[2],NULL,8);

    char fileBuf[255] = {0};
    strcpy(fileBuf,argv[3]);
    if(access(fileBuf,W_OK) != 0){
        perror("access file failed");
        exit(EXIT_FAILURE);
    }

    if(chmod(fileBuf,mt) != 0){
        perror("chmod failed");
        exit(EXIT_FAILURE);
    }


    return 0;
}
