#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>

int main(){
    FILE *fp =popen("rsync 10.136.24.54::search/PC/pic/for_mm/__image__qqface__all__single_new_9.txt","r");
    char buf[1024] = {0};
    fgets(buf,1024,fp);
    pclose(fp);
    printf("buf=%s\tsize=%d\n",buf,strlen(buf));
    return 0;
}
