#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>


int cmd_pwd(int argc, char *argv[]){
    char pwdBuf[255];
    if(getcwd(pwdBuf,255) == NULL){
        perror("operation failed");
        exit(EXIT_FAILURE);
    }
    printf("%s\n",pwdBuf);
    return 0;
}
