#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>


int main(int argc, char* argv[]){
    int ret;
    struct in_addr sip = {0};
    inet_pton(AF_INET, "127.0.0.1", &sip);
    struct sockaddr_in *sin = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    bzero(sin,sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_port = htons(10000);
    sin->sin_addr = sip;

    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd <= 0){
        perror("server socket");
        return -1;
    }
    ret = bind(sd, (struct sockaddr*)sin, sizeof(struct sockaddr));
    if(ret < 0){
        perror("bind");
        return -1;
    }
    if(listen(sd,7) != 0){
        perror("listen");
        return -1;
    }
    struct sockaddr_in sic = {0};
    char buf[50];
    char* ptr = "wendao";
    printf("enter accept.......\n");
    int new_sd;
    while(1){
        bzero(buf,50);
        new_sd = accept(sd, (struct sockaddr*)&sic, &ret);
        if(new_sd <= 0){
            perror("accept");
            return -1;
        }
        ret = 50;
        getpeername(new_sd, (struct sockaddr *)&sic, &ret);
        inet_ntop(AF_INET, &sic.sin_addr, buf, ret);
        printf("---------|%s|---------\n",buf);
        ret = send(new_sd, ptr, strlen(ptr), 0);
        printf("send data size: %d\tnew_sd=%d\n",ret,new_sd);
        recv(new_sd, buf, 50, 0);
        printf("|%s|\n",(char*)buf);
        bzero(buf,50);
    }
    return 0;
}
