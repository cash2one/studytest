#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/file.h>
#include<fcntl.h>
#include "parser_request/thread_func.h"

int main(int argc, char **argv){
	if( argc < 3){
		fprintf(stderr, "USAGE: %s seed_file config_file\n",argv[0]);
		exit(1);
	}
	
	const char* seed_file = argv[1];
	if( access(seed_file,R_OK) != 0 ){
		fprintf(stderr, "read seed file failed\n");
		exit(1);
	}
	const char* config_file = argv[2];
	if( access(config_file,R_OK) != 0 ){
		fprintf(stderr, "read config file failed\n");
		exit(1);
	}
	
	int fd = open(seed_file, O_RDONLY);
	if( fd <= 0 ){
		perror("open seed file failed\n");
		exit(EXIT_FAILURE);
	}
	if( flock(fd, LOCK_EX|LOCK_NB) != 0){
        perror("lock failed");
		fprintf(stderr, "lock seed file failed:%s\n",seed_file);
		exit(EXIT_FAILURE);
	}
	
	start(seed_file, config_file);
	
	flock(fd, LOCK_UN);
	close(fd);
    return 0;

}

