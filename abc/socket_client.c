#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>


int main(int argc, char* argv[]){
    struct in_addr sip;
    inet_pton(AF_INET, "127.0.0.1", &sip);
    struct sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(10000);
    sin.sin_addr = sip;

    int sd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr *sun = (struct sockaddr*)&sin;
    int ret = connect(sd, sun, sizeof(struct sockaddr));
    if(ret != 0){
        perror("connect");
        return -1;
    }
    char buf1[50] = {0};
    ret = recv(sd,buf1,50,0);
    printf("recv data size: %d\t%s\n",ret,buf1);
    char buf[] = "44444444AAAAAAAAAAAAAAAAAA4";
    ret = send(sd, buf, strlen(buf), 0);
    printf("origin data size: %d\tsend data size: %d\n",strlen(buf),ret);
    close(sd);
    return 0;
}
