#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<signal.h>


void func(int signo){
    printf("receive signo %d\n",signo);
}

int main(int argc,char* argv[]){
    pid_t parent_pid = getpid();
    int fd[2];
    pipe(fd);

    pid_t child_pid = fork();
    if(child_pid == 0){//child process
        printf("child process pid=%d\n",getpid());
        printf("child process parent_pid=%d\n",parent_pid);
        close(fd[1]);
        int read_fd = fd[0];
        char buf[100] = {0};
        int i = 0;
        while(1){
            if(read(read_fd,buf,100) > 0){
                printf("child_read:%s\n",buf);
            }
            if((i++) > 60){
                close(read_fd);
            }
        }
    }
    else if(child_pid > 0){//parent process
        printf("parent process pid=%d\n",parent_pid);
        printf("parent process child_pid=%d\n",child_pid);
        signal(SIGPIPE,func);
        close(fd[0]);
        int write_fd = fd[1];
        char buf[100] = {0};
        int i;
        for(i=0; i<100; i++){
            sprintf(buf,"wendao-%d",i);
            write(write_fd,buf,100);
            printf("parent send %d\n",i);
            if(i%10 == 0){
                sleep(2);
            }
        }
        close(write_fd);
    }
    else{
        printf("something error\n");
    }

    return 0;
}
