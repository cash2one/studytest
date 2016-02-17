#include "grab/pageDispatch.h"
#include "utils/Utils.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

/**
 * 开始分了
 */
int32_t pageDispatch::run(){
	if( seed_file.length() <= 0){
		return -1;
	}
	if( access(seed_file.c_str(), R_OK) != 0){
		return -2;
	}
	
	int pagesize = 100;
	FILE *fp = fopen(seed_file.c_str(), "r");
	if(NULL == fp){
		perror("open seed file failed\n");
		return -3;
	}
	
	//遍历循环
	int counter = 1;
	char tmp[1024] = {0};
	while(!feof(fp)){
		fgets(tmp, 1024, fp);
		if(strlen(tmp) <= 0){
			continue;
		}
        string line(tmp);
        Utils::trim(line,"\t\n ");
        int pos  = line.find('\t');
        if(pos == string::npos ){
            continue;
        }
        //已存在
		vector<string> tmp_line;
		Utils::explode(line, "\t", tmp_line);
		if (tmp_line.size() < 3){
			continue;
		}
        string file = tmp_line[2];
        Utils::trim(file,"\t ");
        if( access(file.c_str(),R_OK) == 0){
            continue;
        }
		cur_list.push_back(line);
		if( (counter++) % 100 == 0){
			_queue->addQueue(cur_list);
			cur_list.clear();
		}
		
		bzero(tmp,1024);
	}
	
	
	fclose(fp);
	return 0;
}




