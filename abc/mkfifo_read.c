#include<sys/stat.h>
#include<sys/types.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>


int main(int argc, char* argv[]){
    const char* fifo = "/search/test.fifo";
        FILE *fp = fopen(fifo,"r");
        if(NULL == fp){
            perror(fifo);
            exit(EXIT_FAILURE);
        }
        char buf[1024] = {0};
        printf("------------read process begin\n");
        while(!feof(fp)){
            bzero(buf,1024);
            fread(buf,1024,1,fp);
            printf("%s",buf);
        }
        fclose(fp);
        printf("---------read process finished\n");

    return 0;
}
