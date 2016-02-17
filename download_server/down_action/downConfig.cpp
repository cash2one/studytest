#include "downConfig.h"
#include "utils/Utils.h"
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>

using namespace std;

downConfig::downConfig(const std::string& conf_file):cf(conf_file),share_handle(NULL),headers(NULL)
{
	FILE *fp = fopen(cf.c_str(),"r");
	if(NULL == fp){
		perror("read config file failed");
		exit(EXIT_FAILURE);
	}
	
	

	//遍历文件
	char line[4096] = {0};
	while( !feof(fp) ){
		fgets(line,4096,fp);
		string tmp(line);
		Utils::trim(tmp,"\n\r\t ");
		
		
		//invalid line
		if(tmp.length() <= 0 || tmp.find("=") == string::npos || tmp.at(0) == '#'){
			continue;
		}
		
		
		int ind = tmp.find("=");
		string key = tmp.substr(0,ind);
		string value = tmp.substr(ind+1,tmp.length()-ind-1);
		Utils::trim(key,"\n\r\t\" ");
		Utils::trim(value,"\n\r\t\" ");
		
		
		//一个一个的判断
		if(key.compare("headers[]") == 0 ){ //任务目录
			header_list.push_back(value);
		}
		else if(key.compare("sendfile_shell") == 0){
			sendfile_shell = value;
		}
		else if(key.compare("tmp_html_dir") == 0){
			tmp_html_dir = value;
		}
		else if(key.compare("timeout") == 0){
			timeout = atoi(value.c_str());
		}
		
		
		bzero(line,4096);
	}
	fclose(fp);
	init_env();
}

void downConfig::init_env(){
	list<string>::iterator iter;
	for(iter=header_list.begin(); iter!=header_list.end(); iter++){
		headers = curl_slist_append(headers, (*iter).c_str());
	}
	share_handle = curl_share_init();
	curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);  
}

downConfig::~downConfig(){
	curl_slist_free_all(headers);
	curl_share_cleanup(share_handle );
}





