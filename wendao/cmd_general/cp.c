#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<dirent.h>
#include<sys/types.h>


//不需要额外的参数
//srcFile：源文件/目录
//destFile：目标文件/目录
int cmd_cp(int argc, char *argv[]){
    if(argc < 4){
        fprintf(stderr,"invalid parameter\n");
        exit(EXIT_FAILURE);
    }
    //去掉最后的/
    if(*(argv[2]+strlen(argv[2])-1) == '/'){
        *(argv[2]+strlen(argv[2])-1) = '\0';
    }
    if(*(argv[3]+strlen(argv[3])-1) == '/'){
        *(argv[3]+strlen(argv[3])-1) = '\0';
    }

    //src、dest是否有相应的操作权限
    const char* src = argv[2];
    const char* dest = argv[3];
    if(access(src,R_OK) != 0){
        fprintf(stderr,"source file %s can`t read\n",src);
        exit(EXIT_FAILURE);
    }

    //获取属性信息，目录、文件时的操作不相同
    struct stat srcStat;
    if(stat(src,&srcStat) != 0){
        perror("opertion failed:stat(src)");
        exit(EXIT_FAILURE);
    }
    if(access(dest,F_OK) != 0){
        FILE *fp_tmp=fopen(dest,"w+");
        if(NULL==fp_tmp){
            perror("open file failed");
            exit(EXIT_FAILURE);
        }
        fclose(fp_tmp);
    }
    struct stat destStat;
    if(stat(dest,&destStat) != 0){
        perror("operation failed:stat(dest)");
        exit(EXIT_FAILURE);
    }
    if(S_ISDIR(destStat.st_mode)){
        if(access(dest,W_OK) != 0){
            fprintf(stderr,"dest file %s can`t write\n",dest);
            exit(EXIT_FAILURE);
        }
    }

    //文件拷往目录里
    if(S_ISREG(srcStat.st_mode) && S_ISDIR(destStat.st_mode)){
        char newName[255]={0};
        if(strchr(src,'/') != NULL){
            sprintf(newName,"%s",strrchr(src,'/')+1);
        }
        else{
            sprintf(newName,"%s",src);
        }
        copyFileToDir(src,dest,newName);
    }
    //文件拷到别处
    else if(S_ISREG(srcStat.st_mode)){
        char dirname[255];
        char *rchr = strrchr(dest,'/');
        int i=0;
        for(;i<rchr-dest;i++){
            dirname[i] = *(dest+i);
        }
        copyFileToDir(src,dirname,strrchr(dest,'/')+1);
    }
    //src目录拷往dest目录里:遍历src目录
    else if(S_ISDIR(srcStat.st_mode) && S_ISDIR(destStat.st_mode)){
        recursiveMkdir(src,dest);
    }
    else if(S_ISDIR(srcStat.st_mode) && S_ISREG(destStat.st_mode)){
        fprintf(stderr,"invalid param %s\n",dest);
    }
    else{
        fprintf(stderr,"do nothing\n");
    }
    return 0;
}

int recursiveMkdir(const char *src,const char *dest){
    DIR *dp_src = opendir(src);
    struct dirent *dirItem;
    while((dirItem=readdir(dp_src)) != NULL){
        if(*dirItem->d_name == '.'){
            continue;
        }
        char subName[255]={0};
        sprintf(subName,"%s/%s",src,dirItem->d_name);
        struct stat subStat;
        if( stat(subName,&subStat) != 0 ){
            perror("stat(subStat)");
        }
        //文件处理
        if(S_ISREG(subStat.st_mode)){
            copyFileToDir(subName,dest,dirItem->d_name);
        }
        else if(S_ISDIR(subStat.st_mode)){
            char newSubDir[255];
            sprintf(newSubDir,"%s/%s",dest,dirItem->d_name);
            if(mkdir(newSubDir,0755) != 0){
                perror("mkdir failed");
            }
            recursiveMkdir(subName,newSubDir);
        }
    }
}

int copyFileToDir(const char *filename,const char *dirname,const char *aliasName){
    char newName[255];
    sprintf(newName,"%s/%s",dirname,aliasName);
    FILE *fp_dest = fopen(newName,"w");
    if(NULL == fp_dest){
        perror("open file failed\n");
        exit(EXIT_FAILURE);
    }
    FILE *fp_src = fopen(filename,"r");
    char buf[40960];
    while(!feof(fp_src)){
        fread(buf,40960,1,fp_src);
        fwrite(buf,40960,1,fp_dest);
    }
    fclose(fp_dest);
    fclose(fp_src);
    return 0;
}
