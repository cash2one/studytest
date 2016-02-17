#include "parser_request/parserConfig.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "utils/Utils.h"

/**
 * 解析配置文件，#号为注释符，其他各项格式如下
 * a=1直接被解析
 * b[]=1
 * b[]=2 这样的将被解析成vector存起来
 * seed_file格式： /XX/.../22134.seed
 */
parserConfig::parserConfig(const string& conf_file,  const string& seed_file):
				need_ad(0),task_id(0),page_limit_size(0),html_file_index(2){

	//遍历读取配置文件 
	FILE *fp = fopen(conf_file.c_str(),"r");
	if(NULL == fp){
		perror("read config file failed");
	}
	
	char line[4096] = {0};
	while( !feof(fp) ){
		fgets(line,4096,fp);
		string tmp(line);
		Utils::trim(tmp,"\n\r\t ");
		if(tmp.length() <= 0 || tmp.find("=") == string::npos || tmp.at(0) == '#'){
			continue;
		}
		int ind = tmp.find("=");
		string key = tmp.substr(0,ind);
		string value = tmp.substr(ind+1,tmp.length()-ind-1);
		Utils::trim(key,"\n\r\t\" ");
		Utils::trim(value,"\n\r\t\" ");
		if(key.compare("task_dir") == 0 ){ //任务目录
			task_dir = value;
		}
		else if(key.compare("remote_snapshot_dir") == 0){ //快照目录
			remote_snapshot_dir = value;
		}
		else if(key.compare("local_snapshot_dir") == 0){ //快照目录
			local_snapshot_dir = value;
		}
		else if(key.compare("hosts[]") == 0){ //服务端列表
			hosts.push_back(value);
		}
		else if(key.compare("need_ad") == 0){
			need_ad = atoi(value.c_str());
		}
		else if(key.compare("item_dest_host") == 0){
			item_dest_host = value;
		}
		else if(key.compare("html_dest_host") == 0){
			html_dest_host = value;
		}
		else if(key.compare("mysql_host") == 0){
			mysql_host = value;
		}
		else if(key.compare("mysql_port") == 0){
			mysql_port = value;
		}
		else if(key.compare("mysql_password") == 0){
			mysql_password = value;
		}
		else if(key.compare("mysql_charset") == 0){
			mysql_charset = value;
		}
		else if(key.compare("mysql_user") == 0){
			mysql_user = value;
		}
		else if(key.compare("mysql_db") == 0){
			mysql_db = value;
		}
		else if(key.compare("html_file_index") == 0){
			html_file_index = atoi(value.c_str());
		}
		else if(key.compare("download") == 0){
			download = value;
		}
		
		bzero(line,4096);
	}
	
	fclose(fp);
	
	//提取task_id
	vector<string> tmp;
	Utils::explode(seed_file,"/",tmp);
	string name = tmp[tmp.size()-1];
	Utils::rtrim(name,".seedtmp");
	task_id = atoi(name.c_str());
	
}

//拼凑item文件
int32_t parserConfig::get_item_file(const string& html_file, string& item_file){
	item_file.clear();
	
	///先切
	vector<string> tmp;
	Utils::explode(html_file,"/",tmp);
	if(tmp.size() < 3){
		fprintf(stderr,"html file not absolute path %s\n",html_file.c_str());
		return -2;
	}
	Utils::rtrim(tmp[tmp.size()-1],".html");
	
	Utils::rtrim(task_dir,"/");
	item_file = task_dir;
	item_file.append("/").append(tmp[tmp.size()-2]).append("/").append(tmp[tmp.size()-1]).append(".item");

	return 0;
}

///从html文件里提取引擎编号
int32_t parserConfig::get_engine_id(const string& html_file, string& engine){
	vector<string> tmp;
	Utils::explode(html_file,"/",tmp);
	if(tmp.size() < 3){
		fprintf(stderr,"html file not absolute path %s\n",html_file.c_str());
		return -2;
	}
	
	vector<string> eng;
	Utils::explode(tmp[tmp.size()-2], "_", eng);
	if(eng.size() != 2){
		fprintf(stderr, "get engine_id failed, %s\n",html_file.c_str());
		return -5;
	}
	engine = eng[1];
	return 0;
}


void parserConfig::print_conf(){
    fprintf(stderr,"\n=============parserConfig  START==============\n");
    fprintf(stderr,"task_dir=%s\n",task_dir.c_str());
    fprintf(stderr,"local_snapshot_dir=%s\n",local_snapshot_dir.c_str());
	fprintf(stderr,"remote_snapshot_dir=%s\n",remote_snapshot_dir.c_str());
    vector<string>::iterator iter;
    for(iter=hosts.begin(); iter!=hosts.end();iter++){
        fprintf(stderr,"hosts[]=%s\n",(*iter).c_str());
    }
    fprintf(stderr,"task_id=%d\n",task_id);
    map<string,string>::iterator iter1;
    for(iter1=engine_info.begin(); iter1!=engine_info.end(); iter1++){
        fprintf(stderr,"engine %s\t%s\n",(iter1->first).c_str(),(iter1->second).c_str());
    }
    fprintf(stderr,"need_ad=%d\n",need_ad);
    fprintf(stderr,"mysql_host=%s\n",mysql_host.c_str());
    fprintf(stderr,"mysql_port=%s\n",mysql_port.c_str());
    fprintf(stderr,"mysql_password=%s\n",mysql_password.c_str());
    fprintf(stderr,"mysql_charset=%s\n",mysql_charset.c_str());
    fprintf(stderr,"mysql_user=%s\n",mysql_user.c_str());
    fprintf(stderr,"mysql_db=%s\n",mysql_db.c_str());
    fprintf(stderr,"item_dest_host=%s\n",item_dest_host.c_str());
	fprintf(stderr,"html_dest_host=%s\n",html_dest_host.c_str());
    fprintf(stderr,"download=%s\n",download.c_str());
	fprintf(stderr,"=============parserConfig  END==============\n\n\n");
}

