#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/file.h>
#include<fcntl.h>
#include "utils/ConfigParser.h"
#include "utils/Utils.h"
#include "grab/thread_func.h"
#include "grab/prepare.h"
#include "utils/Global.h"

/**
 * 处理任务的主入口
 * 参数：task_file
 */
int main(int argc, char** argv){
	if( argc < 2 ){
		fprintf(stderr, "invalid param, USAGE:\n");
		fprintf(stderr, "./task ${task_file} > ${task_id}.log\n");
		exit(EXIT_FAILURE);
	}
	
	///处理task_file
	string task_file = argv[1];
	if( access(task_file.c_str(), W_OK) != 0){
		fprintf(stderr, "open task file failed:%s\n",task_file.c_str());
		exit(EXIT_FAILURE);
	}
    int fd = open(task_file.c_str(), O_RDONLY);
	if( fd <= 0 ){
		perror("open task file failed\n");
		exit(EXIT_FAILURE);
	}
	if( flock(fd, LOCK_EX) < 0){
		fprintf(stderr, "lock task file failed:%s\n",task_file.c_str());
		exit(EXIT_FAILURE);
	}
	
	///读取任务文件内容
	char task_info[1024] = {0};
	if(read(fd, task_info, 1024) <= 0 ){
		fprintf(stderr, "read task file failed:%s\n",task_file.c_str());
		exit(EXIT_FAILURE);
	}
	///解析配置文件内容
	string task_info_str = task_info;
	Utils::trim(task_info_str, " \t\n\r");
	vector<string> task_info_vec;
	Utils::explode(task_info_str, "\t", task_info_vec);
	if(task_info_vec.size() != 4){
		fprintf(stderr, "parse task file failed:%s\n",task_file.c_str());
		exit(EXIT_FAILURE);
	}
	
	///读配置文件
	global_task_id = atoi(task_info_vec[0].c_str());
	global_start = atoi(task_info_vec[1].c_str());
	global_limit = atoi(task_info_vec[2].c_str());
	string config_file = task_info_vec[3];
	if( access(config_file.c_str(), R_OK) != 0 ){
		fprintf(stderr, "read config file failed\n");
		exit(EXIT_FAILURE);
	}
	if(global_task_id <= 0 || global_limit <= 0 || global_start < 0){
		fprintf(stderr, "invalid config file content\n");
		exit(EXIT_FAILURE);
	}
	if(task_file.find(task_info_vec[0]) == string::npos){
		fprintf(stderr, "invalid config file content\n");
		exit(EXIT_FAILURE);
	}
	
    ///获取任务文件里的几个字段及锁定之
	global_config.set_config_file(config_file);
	
	
	///生成种子文件及预先检查一下环境
	prepare pre(global_task_id, global_start, global_limit);
	pre.run();

	
	///开始处理，下载完后在TASK_FILE_DIR目录下touch一个 {task_id}.download 文件
	int rem = 1, counter=1;
	while( rem > 0){
		fprintf(stderr, "%d task_start\n", counter++);
		task_start();
		rem = check_download();
		fprintf(stderr, "remain download url num: %d\n", rem);
	}

	flock(fd, LOCK_UN);
	close(fd);
	return 0;
}
