#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<dirent.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<pwd.h>
#include<grp.h>
#include<math.h>
#include<ctype.h>

//快速排序字符串数组
void qsortString(char *strArr[],const int leftIndex,const int rightIndex){
    if(leftIndex >= rightIndex){
        return;
    }
    int start = leftIndex, end=rightIndex;
    char *midStr = strArr[leftIndex];
    while(start < end){
        while(start < end && strcmp(strArr[end],midStr) >= 0){
            end--;
        }
        if(start < end){
           strArr[start++] = strArr[end];
        }
        while(start < end && strcmp(strArr[start],midStr) < 0){
            start++;
        }
        if(start < end){
            strArr[end--]=strArr[start];
        }
    }
    strArr[start]=midStr;
    qsortString(strArr,leftIndex,start-1);
    qsortString(strArr,start+1,rightIndex);
}
//返回如"-rwxr-xr-x"
int parseStatMode(char buf[],int bufsize,mode_t st_mode){
    if(bufsize < 10){
        return -1;
    }
    int i=0;
    for(;i<bufsize;i++){
        buf[i]='-';
    }
    if(S_ISLNK(st_mode)){
        buf[0]='l';
    }
    else if(S_ISREG(st_mode)){
        buf[0]='-';
    }
    else if(S_ISDIR(st_mode)){
        buf[0]='d';
    }
    else if(S_ISCHR(st_mode)){
        buf[0]='c';
    }
    else if(S_ISBLK(st_mode)){
        buf[0]='b';
    }
    else{
        buf[0]='?';
    }
    if(st_mode & S_IRUSR){
        buf[1]='r';
    }
    if(st_mode & S_IWUSR){
        buf[2]='w';
    }
    if(st_mode & S_IXUSR){
        buf[3]='x';
    }
    if(st_mode & S_IRGRP){
        buf[4]='r';
    }
    if(st_mode & S_IWGRP){
        buf[5]='w';
    }
    if(st_mode & S_IXGRP){
        buf[6]='x';
    }
    if(st_mode & S_IROTH){
        buf[7]='r';
    }
    if(st_mode & S_IWOTH){
        buf[8]='w';
    }
    if(st_mode & S_IXOTH){
        buf[9]='x';
    }
    return 0;
}
//由用户号获取用户名称
int uid2name(char unameBuf[],int unameBufSize,uid_t uid,char gnameBuf[],int gnameBufSize){
    struct passwd *pw_ptr = getpwuid(uid);
    if(NULL == pw_ptr){
        perror("uid2name getpwuid");
        exit(EXIT_FAILURE);
    }
    if(unameBufSize < strlen(pw_ptr->pw_name)){
        fprintf(stderr,"not enough space\n");
        exit(EXIT_FAILURE);
    }
    strcpy(unameBuf,pw_ptr->pw_name);
    struct group *gp_ptr = getgrgid(pw_ptr->pw_gid);
    if(NULL == gp_ptr){
        perror("uid2name getgrgid");
        exit(EXIT_FAILURE);
    }
    if(gnameBufSize < strlen(gp_ptr->gr_name)){
        fprintf(stderr,"not enough space\n");
        exit(EXIT_FAILURE);
    }
    strcpy(gnameBuf,gp_ptr->gr_name);
    return 0;
}
//由用户名获取uid、gid
int uname2uid(uid_t uid[],gid_t gid[],const char uname[]){
    if(strlen(uname) <= 0){
        return -1;
    }
    struct passwd *pd = getpwnam(uname);
    if(NULL == pd){
        perror("get passwd failed");
        return -3;
    }
    uid[0] = pd->pw_uid;
    gid[0] = pd->pw_gid;
    return 0;
}
//由组号获取组名称
int gid2gname(char buf[], int bufsize, gid_t gid){
    struct group *gp_ptr = getgrgid(gid);
    strcpy(buf,gp_ptr->gr_name);
    return 0;
}
//去掉字符串两边的特定字符
void trim(char buf[],char ch){
    if(strlen(buf) <= 0){
        return;
    }
    char *str=buf;
    while(*str == ch){
         str += 1;
    }
    int start = 0,diff_len = str - buf;
    int length = strlen(buf) - diff_len;
    for(; start < length; start++){
        buf[start] = buf[start+diff_len];
    }
    length = strlen(str);
    while(*(str+length-1) == ch){
        *(str+length-1)=0;
        length = strlen(str);
    }
}
//检查字符串的内容是否全是某类型字符，如数字、字母
//type = 1：检查此字符串是否全为数字
//type = 2：检查此字符串是否全为字母
//type = 3：检查此字符串是否是由数字、字母组成
int check_string(const char* str,char type){
    if(strlen(str) <= 0){
        return 1;
    }
    int i;
    for(i=0; i<strlen(str); i++){
        switch(type){
            case 1:
                if(!isdigit(*(str+i))){
                    return 1;
                }
                break;
            case 2:
                if(!isalpha(*(str+i))){
                    return 1;
                }
                break;
            case 3:
                if(!isalnum(*(str+i))){
                    return 1;
                }
                break;
            default:
                return 1;
        }
    }
    return 0;
}
