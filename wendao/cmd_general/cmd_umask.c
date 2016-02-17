#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>

int cmd_umask(int argc, char* argv[]){
    mode_t old_mask,new_mask = 022;
    old_mask = umask(&new_mask);
    printf("old_mask=%03o\n",old_mask);
    return 0;
}
