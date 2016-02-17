#include "utils/ConfigParser.h"
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "utils/Utils.h"

ConfigParser::ConfigParser(const string& conf_file):config_file(conf_file){
	parse_conf();
}
ConfigParser::ConfigParser(){
}

/**
 * 获取配置项
 */
int ConfigParser::get_item(const string& key, string& value ){
	if(key.length() <= 0){
		return -1;
	}
	
	map<string,string>::const_iterator iter;
	iter = config.find(key);
	if(iter == config.end()){
		return -2;
	}
	
	value = config[key];
	return 0;
}

/**
 *开始解析配置文件，规则：
 #号开始表示注释行，否则在其他任何地方都被认为是配置项的一部分。
 每个配置项里必须有等号“=”，否则被认为是无效的配置项
 */
int ConfigParser::parse_conf(){
	if( access(config_file.c_str(), R_OK) != 0){
		fprintf(stderr, "access config file %s denied\n", config_file.c_str());
		return -1;
	}
	
	FILE *fp = fopen(config_file.c_str(), "r");
	if(NULL == fp){
		perror("open config file failed");
		return -2;
	}
	
	fprintf(stderr,"start parse config file: %s\n",config_file.c_str());
	
	///一行一行的解析
	config.clear();
	char buf[1024] = {0};
	while( !feof(fp) ){
		bzero(buf,1024);
		fgets(buf,1024,fp);
		if(*buf == '#'){ ///注释
			continue;
		}
		if( strchr(buf,'=') == NULL){ //无效配置
			continue;
		}
		//合法的配置项，存起来
		string item(buf);
		int pos = item.find_first_of('=');
		string key = item.substr(0,pos);
		Utils::trim(key,"\"\n\r\t ");
		string value = item.substr(pos+1,item.length()-pos-1);
		Utils::trim(value,"\"\n\r\t ");
		fprintf(stderr,"parse config item |%s|=|%s|\n",key.c_str(),value.c_str());
		config[key] = value;
	}
	
	fclose(fp);
	return 0;
}

int ConfigParser::set_config_file(const string& conf_file){
	if( access(conf_file.c_str(), R_OK) != 0){
		fprintf(stderr, "access config file %s denied\n", conf_file.c_str());
		return -1;
	}
	config_file = conf_file;
	if(config.size() <=0){
		parse_conf();
	}
	return 0;
}

int ConfigParser::get_config_file(string& conf_file){
    conf_file = config_file;
    return 0;
}

