#include<sys/stat.h>
#include<sys/types.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>
#include<stdint.h>


int main(int argc, char* argv[]){
    const char* fifo = "/search/test.fifo";

        FILE *fp = fopen(fifo,"r+");
        FILE *fp_data = fopen("/search/data/data.test.test","r");
        if(NULL == fp || NULL == fp_data){
            perror(fifo);
            exit(EXIT_FAILURE);
        }
        printf("===============write process begin\n");
        char buf[4096];
        uint64_t num;
        while(!feof(fp_data)){
            bzero(buf,4096);
            fgets(buf,4096,fp_data);
            num = fwrite(buf,1,4096,fp);
            printf("write num=%lu\n",num);
        }
        printf("===============write process finished\n");
        sleep(60);
        fclose(fp);
        fclose(fp_data);

    return 0;
}
