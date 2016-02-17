#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/file.h>
#include<fcntl.h>
#include "down/thread_func.h"

/**
 * 接受一个种子文件作为参数，文件格式
 *  url \t  local_html_file \t ......
 * 即，第一个字段是要下载的url，第二个是下载到本地的什么文件里，其他字段一律忽略
 */
int main(int argc, char** argv){
	if( argc < 2 ){
		fprintf(stderr,"invalid arg,USAGE: \n");
		fprintf(stderr, "%s seed_file\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	
	///处理task_file
	string seed_file = argv[1];
	if( access(seed_file.c_str(), W_OK) != 0){
		fprintf(stderr, "open seed file failed:%s\n",seed_file.c_str());
		exit(EXIT_FAILURE);
	}
    int limit_page_size = 100000; 
    if(argc >= 3){
        limit_page_size = atoi(argv[2]);
    }
    int fd = open(seed_file.c_str(), O_RDONLY);
	if( fd <= 0 ){
		perror("open seed file failed\n");
		exit(EXIT_FAILURE);
	}
	if( flock(fd, LOCK_EX|LOCK_NB) != 0){
        perror("lock failed");
		fprintf(stderr, "lock seed file failed:%s\n",seed_file.c_str());
		exit(EXIT_FAILURE);
	}
	
	///先暴力下载
	task_start_first(seed_file);
	check_download(limit_page_size); //100k以下的文件都有问题
	
	//开始下载，头两次下载不用代理，第三次重试时才用代理
	do{
		task_start_second(seed_file);
	}while(check_download(10) > 0);
	

	flock(fd, LOCK_UN);
	close(fd);
	return 0;
}
