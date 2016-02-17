#include<sys/stat.h>
#include<sys/types.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>


int main(int argc, char* argv[]){
    const char* fifo = "/search/test.fifo";
    if(access(fifo,R_OK) !=0){
        if(mkfifo(fifo,0755) != 0){
            perror(fifo);
            exit(EXIT_FAILURE);
        }
    }

    pid_t cpid = fork();
    if(cpid == 0){
        FILE *fp = fopen(fifo,"r");
        if(NULL == fp){
            perror(fifo);
            exit(EXIT_FAILURE);
        }
        char buf[1024] = {0};
        sleep(5);
        printf("------------read process begin\n");
        while(!feof(fp)){
            bzero(buf,1024);
            fread(buf,1024,1,fp);
            printf("%s",buf);
        }
        fclose(fp);
        printf("---------read process finished\n");
        if(fork() == 0){
            printf("+++++++++++++++sub sub process++++++++++++++++++\n");
            FILE *fp1 = fopen(fifo,"r");
            char buf1[1024] = {0};
            printf("------------begin\n");
            while(!feof(fp1)){
                bzero(buf1,1024);
                fread(buf1,1024,1,fp1);
                printf("%s",buf1);
            }
            fclose(fp1);
        }
    }
    else if(cpid > 0){
        FILE *fp = fopen(fifo,"r+");
        FILE *fp_data = fopen("/search/data/data.test.test","r");
        if(NULL == fp || NULL == fp_data){
            perror(fifo);
            exit(EXIT_FAILURE);
        }
        printf("===============write process begin\n");
        char buf[4096];
        while(!feof(fp_data)){
            bzero(buf,4096);
            fgets(buf,4096,fp_data);
            fwrite(buf,4096,1,fp);
        }
        fclose(fp);
        fclose(fp_data);
        printf("===============write process finished\n");
    }
    else{
        printf("something error occur\n");
    }

    return 0;
}
