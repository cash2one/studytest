#include "grab/prepare.h"
#include "utils/MD5.h"
#include "utils/URLEncoder.h"
#include "utils/ConfigParser.h"
#include<unistd.h>
#include<time.h>
#include<fcntl.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include "utils/Global.h"

/**
 * 主函数
 */
int prepare::run(){
	db.initDB();
	
	///获取应用信息
	int task_ret = db.get_task_info(task_id,task_info);
	if(task_info.size() <= 0){
		fprintf(stderr, "get task info failed, task_id=%d\treturn code=%d\n", task_id, task_ret);
		exit(EXIT_FAILURE);
	}
	
	///检查运行环境
	check_env();
	
	//产生种子文件
	generate_seed();
	
	return 0;
}

/**
 * 检查各种运行环境
 */
int prepare::check_env(){
	///检查日志目录是否可写
	string log_path;
	global_config.get_item("LOG_FILE_DIR", log_path);
	if( access(log_path.c_str(), W_OK) != 0){
		fprintf(stderr,"write log path failed\n");
		exit(EXIT_FAILURE);
	}
	
	
	///检查存放种子的目录是否可写
	string seed_path;
	global_config.get_item("SEED_FILE_DIR", seed_path);
	if( access(seed_path.c_str(), W_OK) != 0){
		fprintf(stderr,"write seed dir failed\n");
		exit(EXIT_FAILURE);
	}
	
	
	///检查存放下载的网页的目录是否可写
	string page_path;
	global_config.get_item("LOCAL_PAGE_DIR", page_path);
	if( access(page_path.c_str(), W_OK) != 0){
		fprintf(stderr,"write page dir failed\n");
		exit(EXIT_FAILURE);
	}

	
	///检查存放任务目录是否存在且可写
	string task_path;
	global_config.get_item("TASK_FILE_DIR", task_path);
	if( access(task_path.c_str(), W_OK) != 0){
		fprintf(stderr,"write task dir failed\n");
		exit(EXIT_FAILURE);
	}
	
	
	///生成存放网页的子目录，格式：2014-07/1923_3/$md5.html
	Utils::rtrim(page_path, '/');
	time_t ctime = atol(task_info["createtime"].c_str());
	struct tm *ct = localtime(&ctime);
	char dt[10] = {0};
	sprintf(dt,"%d-%02d", ct->tm_year + 1900, ct->tm_mon + 1);
	page_path.append("/").append(dt);
	if( access(page_path.c_str(), W_OK) != 0){
		if(mkdir(page_path.c_str(), 0755) != 0){
			fprintf(stderr,"create page dir failed: %s\n", page_path.c_str());
			exit(EXIT_FAILURE);
		}
	}
	
	
	///引擎列表，以\t隔开的
	string engine = task_info["engine"];
	vector<string> eid_list;
	get_eid_list(engine, eid_list);
	vector<string>::iterator iter;
	for(iter=eid_list.begin(); iter!=eid_list.end(); iter++){
		string tmp_page_path = page_path;
		tmp_page_path.append("/").append(task_info["tid"]).append("_").append(*iter);
		if( access(tmp_page_path.c_str(), W_OK) != 0){
			if(mkdir(tmp_page_path.c_str(), 0755) != 0){
				fprintf(stderr,"create page dir failed: %s\n", tmp_page_path.c_str());
				exit(EXIT_FAILURE);
			}
		}
		///当前任务，哪个引擎的目录先留着
		engine_local_file[*iter] = tmp_page_path;
	}
	
	return 0;
}

/**
 * 产生种子文件
 */
int prepare::generate_seed(){
	///拼装引擎参数
	map<string,vector<string> > where_engine;
	vector<string> param2;
	get_eid_list(task_info["engine"], param2);
	where_engine["eid:in"] = param2;
	map<int, map<string, string> > engine_list;
	map<string, string> orderby;
	string format_field;
	db.getList("tb_engine", where_engine, 0, param2.size(), engine_list );
	if(engine_list.size() <= 0){
		fprintf(stderr,"get engine info failed\n");
		exit(EXIT_FAILURE);
	}
	
	
	///种子文件
	string seed_path;
	global_config.get_item("SEED_FILE_DIR", seed_path);
	seed_path.append("/").append(task_info["tid"]).append(".seed");
	if( access(seed_path.c_str(), R_OK) == 0){ //若种子文件已存在了
		return 0;
	}
	FILE *fp = fopen(seed_path.c_str(),"a+");
	if(NULL == fp){
		perror("create seed file failed");
		exit(EXIT_FAILURE);
	}
	
	///分页获取query列表
	int arg_start = start;
	int pagesize = 100;
	
	///where参数
	map<string,vector<string> > where_query;
	vector<string> param1;
	param1.push_back(task_info["tid"]);
	where_query["tid:eq"] = param1;
	
	map<int, map<string, string> > query_list;
	map<int, map<string, string> >::iterator query_iter, engine_iter;
	
	MD5 md5;
	
	///开始分页查询
	do{
		///防止获取过多的数据
		if(arg_start + pagesize >= start + limit){
			pagesize = start + limit - arg_start;
		}
		if(pagesize <= 0){
			break;
		}
		db.getList("tb_dcg_query", where_query, arg_start, pagesize, query_list );
		if(query_list.size() <= 0){
			break;
		}
		
		///遍历之，格式：query \t engine_url \t file_name
		for(query_iter=query_list.begin(); query_iter != query_list.end(); query_iter++){
			for(engine_iter=engine_list.begin(); engine_iter!=engine_list.end(); engine_iter++){
				///拼装URL，对query进行转码处理
				string url = (engine_iter->second)["url"];
				string query = (query_iter->second)["query"];
				char tmp[1024] = {0};
				URLEncoder::encoding(query.c_str(), tmp,1024);
				///将要在本地存的文件
				string local_path = engine_local_file[(engine_iter->second)["eid"]];
				md5.reset();
				md5.update((query_iter->second)["query"]);
				local_path.append("/").append(md5.toString()).append(".html");
				///合并，写入种子文件
				string last_line = tmp;
				last_line.append("\t").append(url).append("\t").append(local_path).append("\n");
				fwrite(last_line.c_str(), last_line.length(), 1, fp);
			}
		}
		
		arg_start += query_list.size();
		query_list.clear();
	}while(1);
	
	fclose(fp);
	return 0;
}

/**
 * 从以\t隔开的字符串里获取eid列表
 */
int prepare::get_eid_list(const string& eidstr, vector<string>& eids){
	string eid_str = eidstr;
	Utils::trim(eid_str, "\t ");
	if(eid_str.length() <= 0){
		return -2;
	}
	eids.clear();
	
	///遍历之
	while(eid_str.length() > 0){
		int eid = atoi(eid_str.c_str());
		char e[10] = {0};
		sprintf(e,"%d",eid);
		eids.push_back(e);
		eid_str = eid_str.substr(strlen(e),eid_str.length()-strlen(e));
		Utils::trim(eid_str, "\t ");
	}
	
	return 0;
}




