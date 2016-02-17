#include "down/pageDispatch.h"
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
		perror("[pageDispatch::run] open seed file failed\n");
		return -3;
	}
	
	//遍历循环
	int counter = 1;
	char tmp[4096] = {0};
	while(!feof(fp)){
		fgets(tmp, 4096, fp);
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
		if (tmp_line.size() < 2){
			continue;
		}
		string url = tmp_line[0];
        string file = tmp_line[1];
        if( access(file.c_str(),R_OK) == 0){
            continue;
        }
		url.append("\t").append(file);
		cur_list.push_back(url);
		if( (counter++) % 100 == 0){
			_queue->addQueue(cur_list);
			cur_list.clear();
		}
		bzero(tmp,4096);
	}
	if(cur_list.size() > 0){
		_queue->addQueue(cur_list);
		cur_list.clear();
	}
	
	fclose(fp);
	fprintf(stderr,"[pageDispatch::run] dispatch thread exit: %lu\n", pthread_self());
	return 0;
}




