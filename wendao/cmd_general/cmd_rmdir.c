#include<dirent.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>

static int rmDir(const char* dirname);

int cmd_rmdir(int argc, char *argv[]){
    if(argc < 3){
        fprintf(stderr,"wrong number of paramters\n");
        exit(EXIT_FAILURE);
    }

    const char * dest_dir = argv[2];
    if(access(dest_dir,W_OK) != 0){
        fprintf(stderr,"operation failed\n");
        exit(EXIT_FAILURE);
    }
    rmDir(dest_dir);

    return 0;
}

static int rmDir(const char * dirname){
    //去掉目录尾部的/
    char tmp_dname[strlen(dirname)];
    bzero(tmp_dname,strlen(dirname));
    strcpy(tmp_dname,dirname);
    if('/' == tmp_dname[strlen(dirname)-1]){
        tmp_dname[strlen(dirname)-1] = '\0';
    }

    //打开目录，读取其下的文件
    DIR *dp = opendir(tmp_dname);
    if(NULL == dp){
        perror("open dir failed");
        return 1;
    }
    struct dirent *tmp_dir;
    char buf[255] = {0};
    struct stat st;
    while((tmp_dir=readdir(dp)) != NULL){
        if(strcmp(tmp_dir->d_name,".") == 0 || strcmp(tmp_dir->d_name,"..") == 0){
            continue;
        }
        bzero(buf,255);
        sprintf(buf,"%s/%s",tmp_dname,tmp_dir->d_name);
        if(stat(buf,&st) != 0){
            perror("read file stat failed");
            return 2;
        }
        //处理文件
        if(S_ISDIR(st.st_mode)){
            rmDir(buf);
        }
        else if(remove(buf) != 0){
            perror("delete file failed");
            return 3;
        }
    }
    rmdir(tmp_dname);
    closedir(dp);
    return 0;
}
