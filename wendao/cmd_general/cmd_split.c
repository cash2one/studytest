#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>

static void split_file_by_bytes(const char* filename, const char* suffix,uint64_t split_size);
static void split_file_by_line(const char* filename,const char* suffix,uint64_t split_size);

#define TRUE 1
#define FALSE 0

//可以按行、按字节分隔文件
//-l：按行分隔
//-b：按字节分隔
//-s：分隔文件的后缀
int cmd_split(int argc, char *argv[]){
    if(argc < 3){
        fprintf(stderr,"invalid number of parameters\n");
        return 1;
    }

    //第一个不以-开头的认为是文件
    char filename[255] = {0};
    int i;
    for(i=2; i< argc; i++){
        if(*argv[i] != '-'){
            strcpy(filename,argv[i]);
        }
    }
    if(access(filename,R_OK) != 0){
        perror(filename);
        return 2;
    }

    //读取参数，分隔文件
    char ch;
    uint8_t split_type = 0;
    uint64_t split_size = 0;
    char suffix[50]={0};
    while((ch=getopt(argc,argv,"l:b:s:")) != -1){
        switch(ch){
            case 'l':
                split_type = 1;
                split_size=strtoul(optarg,NULL,10);
                break;
            case 'b':
                split_type = 2;
                split_size=strtoul(optarg,NULL,10);
                break;
            case 's':
                strcpy(suffix,optarg);
                break;
        }
    }
    fprintf(stderr,"filename=%s\n",filename);
    fprintf(stderr,"suffix=%s\n",suffix);
    fprintf(stderr,"split_size=%lu\n",split_size);
    if(1 == split_type){
        split_file_by_line(filename,suffix,split_size);
    }
    else if(2 == split_type){
        split_file_by_bytes(filename,suffix,split_size);
    }
    else{
        fprintf(stderr,"invalid paramter\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr,"operation success\n");
    return 0;
}
//按行分隔文件，split_size是每个文件的行数，suffix是分隔文件后的后缀
static void split_file_by_line(const char* filename,const char* suffix,uint64_t split_size){
    FILE *fp = fopen(filename,"r");
    if(NULL == fp){
        perror(filename);
        exit(EXIT_FAILURE);
    }
    FILE *sfp = NULL;
    char lineBuf[10240]={0};
    char sub_name[255]={0};
    uint64_t line_counter = 0;
    uint32_t counter = 0;
    while(!feof(fp)){
        if((line_counter++) % split_size == 0){
            bzero(sub_name,255);
            while(TRUE){
                sprintf(sub_name,"%s_%s_%06d",filename,suffix,counter++);
                if(access(sub_name,R_OK) != 0){
                    break;
                }
            }
            if(NULL != sfp){
                fclose(sfp);
            }
            sfp = fopen(sub_name,"w+");
            if(NULL == sfp){
                perror(sub_name);
                exit(EXIT_FAILURE);
            }
        }
        fgets(lineBuf,10240,fp);
        if(fputs(lineBuf,sfp) == EOF){
            perror("write file failed");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr,"read %lu lines\n",line_counter);
    }
    fclose(fp);
    if(NULL != sfp){
        fclose(sfp);
    }
}

//按字节分隔文件，split_size是每个文件的字符数，suffix是分隔的后缀
static void split_file_by_bytes(const char* filename, const char* suffix,uint64_t split_size){
    FILE *fp = fopen(filename,"r");
    if(NULL == fp){
        perror(filename);
        exit(EXIT_FAILURE);
    }

    FILE *sfp = NULL;
    char bytesBuf[10240] = {0};
    uint32_t counter = 0,read_counter = 0;
    char sub_name[255] = {0};
    uint64_t bytes_counter = 0,cur_counter = 0,read_num=0; //一共读的字节数、写入本sub_name的字节数
    while(!feof(fp)){
        bzero(bytesBuf,10240);
        if(split_size == cur_counter || cur_counter == 0){
            bzero(sub_name,255);
            while(TRUE){
                sprintf(sub_name,"%s_%s_%06d",filename,suffix,counter++);
                if(access(sub_name,R_OK) != 0){
                    break;
                }
            }
            if(NULL != sfp){
                fclose(sfp);
            }
            sfp = fopen(sub_name,"w+");
            if(NULL == sfp){
                perror("write file failed");
                exit(EXIT_FAILURE);
            }
            fprintf(stderr,"%s\n",sub_name);
            cur_counter = 0;
        }
        if(split_size >= cur_counter + 10240){
            read_counter = 10240;
        }
        else{
            read_counter = split_size - cur_counter;
            if(read_counter <= 0){
                continue;
            }
        }
        read_num = fread(bytesBuf,1,read_counter,fp);
        fprintf(stderr,"read_num=%lu\tbytesBuf=%s\n",read_num,bytesBuf);
        fwrite(bytesBuf,1,read_num,sfp);
        cur_counter += read_num;
        bytes_counter += cur_counter;
        fprintf(stderr,"bytes_counter=%lu\tcur_counter=%lu\tread_counter=%d\n",bytes_counter,cur_counter,read_counter);
    }

    fclose(fp);
    if(NULL != sfp){
        fclose(sfp);
    }
}
