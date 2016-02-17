#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdint.h>
#include<string.h>


//统计文件的行数、字节数。每次读一大块数据，然后统计里面的\n的数量就可以了
//-l：行数
//-b：字节数
int cmd_wc(int argc, char* argv[]){
    if(argc < 4){
        fprintf(stderr,"wrong number of parameter\n");
        exit(EXIT_FAILURE);
    }

    //获取文件
    char fileBuf[255] = {0};
    strcpy(fileBuf,argv[3]);
    if(access(fileBuf,R_OK) != 0){
        perror(fileBuf);
        exit(EXIT_FAILURE);
    }

    //接收参数
    int ch;
    char type;
    while((ch=getopt(argc,argv,"lb")) != -1){
        switch(ch){
            case 'l':type = 1;break;
            case 'b':type = 2;break;
        }
    }

    //开始统计
    if(1 == type){ //按行统计
        char buf[4194304] = {0};
        uint32_t counter = 0;
        FILE *fp = fopen(fileBuf,"r");
        char* p = NULL;
        char* q = NULL;
        if(NULL == fp){
            perror(fileBuf);
            exit(EXIT_FAILURE);
        }
        int i;
        while(!feof(fp)){
            bzero(buf,4194304);
            fread(buf,4194304,1,fp);
            p = buf;
            while((q=strchr(p,'\n')) != NULL){
                counter++;
                p = q + 1;
            }
        }
        fclose(fp);
        fprintf(stderr,"%u\t%s\n",counter,fileBuf);
    }
    else if(2 == type){ //按字节统计
        struct stat st;
        if(stat(fileBuf,&st) != 0){
            perror(fileBuf);
            exit(EXIT_FAILURE);
        }
        fprintf(stderr,"%lu\t%s\n",st.st_size,fileBuf);
    }
    else{
        fprintf(stderr,"invalid para\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
