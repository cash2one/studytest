#include "parser_request/parserDispatch.h"
#include "utils/Utils.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>


/**
 * 开始分了
 */
int32_t parserDispatch::run(){
	if( insert_seed_list() != 0){
		fprintf(stderr,"[parserDispatch::run] insert_seed_list failed,something is wrong\n");
		return -3;
	}
	
	if( seed_list.size() <= 0){
		fprintf(stderr,"[parserDispatch::run] seed list empty\n");
		return -3;
	}
	
	///开始遍历
	list<string>::iterator iter;
	while( seed_list.size() > 0){
		int counter = 1;
		
		fprintf(stderr,"current seed list size:%d\n",seed_list.size());
		
		//遍历所有的文件
		for(iter=seed_list.begin();  iter != seed_list.end();){
			string line = *iter;
			vector<string> tmp_line;
			Utils::explode(line, "\t", tmp_line);
			if(tmp_line.size() < 3){
				fprintf(stderr,"tmp_line.size() < 3\n");
				iter++;
				continue;
			}
			
			//提取html_file及item_file
			string html_file = tmp_line[1];
			string item_file;
			conf->get_item_file(html_file, item_file);
			
			//已经解析过了
			if( access(item_file.c_str(),R_OK) == 0){
				seed_list.erase(iter++);
				fprintf(stderr,"already exists %s\n",item_file.c_str());
				continue;
			}
			
			//放到待解析队列里
			cur_list.push_back(line);
			if( (counter++) % 200 == 0){
				_queue->addQueue(cur_list);
				cur_list.clear();
			}
		}
		
		//剩下的全部放进去
		if(cur_list.size() > 0){
			_queue->addQueue(cur_list);
			cur_list.clear();
		}
		sleep(1);
	}

	fprintf(stderr,"[parserDispatch::run] thread exit: %lu\n", pthread_self());
	return 0;
}

///填充seed_list，以后遍历的时候只遍历它就行了
int32_t parserDispatch::insert_seed_list(){
	if( seed_file.length() <= 0){
		return -1;
	}
	if( access(seed_file.c_str(), R_OK) != 0){
		fprintf(stderr,"[parserDispatch::insert_seed_list] read seed file failed,%s\n",seed_file.c_str());
		return -2;
	}
	//遍历文件
	FILE *fp = fopen(seed_file.c_str(), "r");
	if(NULL == fp){
		perror("[parserDispatch::insert_seed_list] open seed file failed\n");
		return -3;
	}

	//遍历循环
	char tmp[4096] = {0};
	while(!feof(fp)){
		fgets(tmp, 4096, fp);
		if(strlen(tmp) <= 0){
			fprintf(stderr,"fgets failed\n");
			continue;
		}
        string line(tmp);
        Utils::trim(line,"\t\n ");
        int pos  = line.find("\t");
        if(pos == string::npos ){
			fprintf(stderr,"fint delim failed\n");
            continue;
        }
		
		vector<string> tmp_line;
		Utils::explode(line, "\t", tmp_line);
		if (tmp_line.size() < 3){
			fprintf(stderr,"item size < 3\n");
			continue;
		}
		
		seed_list.push_back(line);		
		bzero(tmp,4096);
	}

	
	fclose(fp);
	return 0;
}

//通过锁定种子文件的办法判断下载进程是否还存在
int32_t parserDispatch::check_down_finish(){
	int fd = open(seed_file.c_str(), O_RDONLY);
	int ret = flock(fd, LOCK_EX|LOCK_NB);
	if( ret == 0){
		flock(fd, LOCK_UN);
	}
	else{
		perror("[parserDispatch::check_down_finish] lock seed file failed,");
	}
	close(fd);
	return ret;
}

