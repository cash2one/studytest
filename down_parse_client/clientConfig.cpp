#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include "Utils.h"
#include "MD5.h"
#include "clientConfig.h"
#include "down_client/get_proxy_list.h"
#include "jsoncpp/jsoncpp_json.h"


using namespace std;


/**构造函数，直接读取配置文件里的内容**/
clientConfig::clientConfig(const string& conf_file,const std::string& tid): \
				cf(conf_file),page_counter(0),task_id(tid),parse_thread_exit(0),is_retry(0),db(NULL)
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
		bzero(line,4096);
		
		
		//invalid line
		if(tmp.length() <= 0 || tmp.find("=") == string::npos || tmp.at(0) == '#'){
			continue;
		}
		
		
		int ind = tmp.find("=");
		string key = tmp.substr(0,ind);
		string value = tmp.substr(ind+1,tmp.length()-ind-1);
		Utils::trim(key,"\n\r\t\" ");
		Utils::trim(value,"\n\r\t\" ");
        if(value.length() <= 0 || key.length() <= 0){
            continue;
        }
		
		
		//一个一个的判断
		if(key.compare("mysql_host") == 0 ){
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
		else if(key.compare("snapshot_dir") == 0){
			snapshot_dir = value;
		}
		else if(key.compare("task_dir") == 0){
			task_dir = value;
		}
		else if(key.compare("proxy_api") == 0){
			proxy_api = value;
		}
		else if(key.compare("normal_page_limit_size") == 0){
			normal_page_limit_size = value;
		}
		else if(key.compare("page_num_per_proxy") == 0){
			page_num_per_proxy = value;
		}
		else if(key.compare("download_hosts[]") == 0){ /**这个是可以重复多次的**/
			parseList(download_hosts,value);
		}
		else if(key.compare("parse_hosts[]") == 0){
			parseList(parse_hosts,value);
		}
		else if(key.compare("sogou_parse_hosts[]") == 0){
			parseList(sogou_parse_hosts,value);
		}
		else if(key.compare("dest_host") == 0){
			dest_host = value;
		}
	}
	fclose(fp);
	
	
	/**拼凑seed文件**/
	seed_file = task_dir;
	Utils::rtrim(seed_file,"/");
	seed_file.append("/").append(task_id).append(".seed");
	
	
	/**连接数据库**/
	db = new mark_oss(mysql_host, mysql_user, mysql_db, mysql_password, mysql_charset,atoi((mysql_port).c_str()));
}



/**更改任务的状态标志**/
void clientConfig::set_task_status(const string& status){
	string table ="tb_dcg_task";
	map<string,string> update_data;
	update_data["status"] = status;
	map<string,vector<string> > where;
	vector<string> tmp;
	tmp.push_back(task_id);
	where["tid"] = tmp;
	db->update(table,update_data,where);
}



/**主要工作：从MySQL里查询引擎信息、产生种子文件等**/
void clientConfig::generateInfo(){
	if( task_id.length() <= 0){
		fprintf(stderr,"[clientConfig::generateInfo] invalid task_id[%s]\n",task_id.c_str());
		return;
	}
	if( access(task_dir.c_str(), W_OK) != 0){
		if(mkdir(task_dir.c_str(), 0755) != 0){
			fprintf(stderr,"create page dir failed: %s\n", task_dir.c_str());
			return;
		}
	}
	
	
	/**查询任务详细信息**/
	map<string,string> task_info;
	db->get_task_info(atoi(task_id.c_str()), task_info);
	if( task_info.size() <= 0){
		fprintf(stderr,"[clientConfig::generateInfo] get task info failed,task_id=%s\n",task_id.c_str());
		return;
	}
	
	
	
	/**engine info**/
	vector<string> eid;
	Utils::explode(task_info["engine"], "\t", eid);
	map<string,vector<string> > where;
	where["eid:in"] = eid;
	map<string, string> orderby;
	map<string, map<string, string> > engine_list;
	db->getList("tb_engine", where, 1, eid.size(), "eid", orderby, engine_list);
	if( engine_list.size() <= 0){
		fprintf(stderr,"[clientConfig::generateInfo] get engine list failed,engine ids %s\n",(task_info["engine"]).c_str());
		return;
	}
	
	
	
	
	/**格式化引擎信息**/
	map<string, map<string, string> >::iterator iter;
	for(iter=engine_list.begin(); iter!=engine_list.end(); iter++){
		string ename = (iter->second)["name"];
		string eid_str = (iter->second)["eid"];
		
		//sogou、 baidu、360一个一个的判断
		if(ename.find("sogou") != string::npos){
			(engine_info)[eid_str] = "sogou-no-ad";
		}
		else if(ename.find("baidu") != string::npos){
			(engine_info)[eid_str] = "baidu";
		}
		else if(ename.find("360") != string::npos){
			(engine_info)[eid_str] = "360so";
		}
		else{
			fprintf(stderr, "[clientConfig::generateInfo] unknown engine name %s", ename.c_str());
			return;
		}
		
		/**创建任务目录**/
		char task_tmp[255] = {0};
		sprintf(task_tmp,"%s/%s_%s",task_dir.c_str(),task_id.c_str(),eid_str.c_str());
		
		/**完全重试的**/
		if( 1 == is_retry ){
			string cmd = "rm -rf ";
			cmd.append(task_tmp);
			system(cmd.c_str());
		}
		
		/**创建任务的目录**/
		if( access(task_tmp, W_OK) != 0){
			if(mkdir(task_tmp, 0755) != 0){
				fprintf(stderr,"create task dir failed: %s\n", task_tmp);
				return;
			}
		}
	}
	
	
	
	
	/**分页获取query拼凑种子文件**/
	int pagesize = 1000;
	string query_anch = "__QUERY__";
	string table = "tb_dcg_query";
	map<string,vector<string> > where_query;
	vector<string> param1;
	param1.push_back(task_id);
	where_query["tid:eq"] = param1;
	string format_field;
	map<string, string> orderby_query;
	
	
	/**操作查询结果的**/
	map<string, map<string, string> > ret;
	map<string, map<string, string> >::iterator iter_ret;
	
	
	/**形如：2014-08**/
	string snapshot = snapshot_dir;
	Utils::rtrim(snapshot,"/");
	time_t ctime = atol(task_info["createtime"].c_str());
	struct tm *ct = localtime(&ctime);
	char dt[10] = {0};
	sprintf(dt,"%d-%02d", ct->tm_year + 1900, ct->tm_mon + 1);
	snapshot.append("/").append(dt);
	if( access(snapshot.c_str(), W_OK) != 0){
		if(mkdir(snapshot.c_str(), 0755) != 0){
			fprintf(stderr,"create page dir failed: %s\n", snapshot.c_str());
			return;
		}
	}
	
	
	/**计算md5用的**/
	MD5 md5;
	
	
	FILE *fp = fopen(seed_file.c_str(),"w");
	if(NULL == fp){
		perror("create seed file failed");
		return;
	}
	
	
	/**开始查询**/
	int page = 1;
	do{
		ret.clear();
		/**分页查询**/
		db->getList(table, where_query, page++, pagesize,format_field, orderby_query, ret );
		for(iter_ret=ret.begin(); iter_ret!=ret.end(); iter_ret++){
			for(iter=engine_list.begin(); iter!=engine_list.end(); iter++){
				string eurl = (iter->second)["url"];
				string eid_str = (iter->second)["eid"];
				string query = (iter_ret->second)["query"];
				
				/**对query进行编码，并替换eurl中的占位符**/
				string encode_query;
				Utils::UrlEncode(query, encode_query);
				eurl.replace(eurl.find(query_anch),   query_anch.length(),   encode_query);
				
				/**计算md5值**/
				md5.reset();
				md5.update(query);
				
				/**预先创建快照目录**/
				string snap = snapshot;
				snap.append("/").append(task_id).append("_").append(eid_str);
				
				/**完全重试**/
				if( 1 == is_retry ){
					string cmd = "rm -rf ";
					cmd.append(snap);
					system(cmd.c_str());
				}
				
				/**建立快照目录**/
				if( access(snap.c_str(), W_OK) != 0){
					if(mkdir(snap.c_str(), 0755) != 0){
						fprintf(stderr,"create page dir failed: %s\n", snap.c_str());
						return;
					}
				}
				
				/**拼种子文件的每一行**/
				char tmp_line[4096] = {0};
                snap.append("/").append(md5.toString()).append(".html");
				sprintf(tmp_line,"%s\t%s\t%s\n",eurl.c_str(),snap.c_str(),query.c_str());
				fwrite(tmp_line, strlen(tmp_line), 1, fp);
				page_counter++;
			}
		}
	}while(ret.size() > 0);
	fclose(fp);
	
	
	
	if( 1 == is_retry ){  /**完全重试的**/
		vector<string> param;
		param.push_back(task_id);
		string sql = "DELETE FROM `tb_dcg_data_list` WHERE `task_id`=?";
		db->execute(sql,param);
	}
	else if (2 == is_retry){ /**部分重试**/
		check_parse_fail();
	}
}


/**从html_file里提取一些信息**/
void clientConfig::get_engine_item(const string& html_file, string& engine_id, string& item_file){
	/**先切的一段一段的**/
	vector<string> tmp;
	Utils::explode(html_file,"/",tmp);
	if(tmp.size() < 3){
		fprintf(stderr,"html file not absolute path %s\n",html_file.c_str());
		return;
	}

	
	/**先获取engine**/
	vector<string> eng;
	Utils::explode(tmp[tmp.size()-2], "_", eng);
	if(eng.size() != 2){
		fprintf(stderr, "get engine_id failed, %s\n",html_file.c_str());
		return;
	}
	engine_id = eng[1];
	
	
	/**拼装item_file地址**/
	Utils::rtrim(tmp[tmp.size()-1],".html");
	
	item_file = task_dir;
	item_file.append("/").append(tmp[tmp.size()-2]).append("/").append(tmp[tmp.size()-1]).append(".item");
}


/**更新代理**/
void clientConfig::update_proxy_list(){
	/**若全是sogou的就不用代理了**/
	int is_sogou_engine = 1;
	map<string,string>::iterator iter_map;
	for(iter_map=engine_info.begin(); iter_map!=engine_info.end(); iter_map++){
		if( (iter_map->second).find("sogou") == string::npos ){
			is_sogou_engine = 0;
		}
	}
	if( is_sogou_engine > 0 ){
		return;
	}
	
	
	/**要提取的代理IP的数量**/
	int num = page_counter / atoi(page_num_per_proxy.c_str());
	num = (num > 100) ? 100 : num;
	get_proxy_list proxy_obj(num, this);
	proxy_obj.request(proxy_list);
}



/**解析一些列表页**/
void clientConfig::parseList(vector<string>& tl, const string& value){
	if(value.find(":") == string::npos){
		tl.push_back(value);
		return;
	}
	vector<string> tmp;
	Utils::explode(value,":",tmp);
	if(tmp.size() >= 2){
		string item1 = tmp[0];
		item1.append(":").append(tmp[1]);
		int num = 1;
		if(tmp.size() >= 3){
			num = atoi(tmp[2].c_str());
		}
		for(int i=0; i<num; i++){
			tl.push_back(item1);
		}
	}
}


/**检查解析失败的**/
void clientConfig::check_parse_fail(){

	/**提取所有解析失败的query**/
	string sql = "SELECT query FROM tb_dcg_query WHERE tid =? AND query NOT IN (SELECT DISTINCT query FROM tb_dcg_data_list WHERE task_id=?)";
	vector<string> param;
	param.push_back(task_id);
	param.push_back(task_id);
	map<int,map<string,string> > ret;
	db->fetch_all(sql, param, ret);
	if( ret.size() <= 0){
		return;
	}
	
	
	/**然后将它们的item文件干掉，以便重新解析**/
	MD5 md5;
	map<int,map<string,string> >::iterator iter;
	map<string, string>::iterator iter_eng;
	for(iter=ret.begin(); iter!=ret.end(); iter++){
		string query = (iter->second)["query"];
		md5.reset();
		md5.update(query);
		string md5v = md5.toString();
		for(iter_eng=engine_info.begin(); iter_eng!=engine_info.end(); iter_eng++){
			string eid = iter_eng->first;
			string item_file = task_dir;
			item_file.append("/").append(task_id).append("_").append(eid).append("/").append(md5v).append(".item");
			if(unlink(item_file.c_str()) != 0){
				fprintf(stderr,"[clientConfig::check_parse_fail] unlink item file failed %s\n",item_file.c_str());
			}
		}
	}

}



/**解析好的json直接导入数据库**/
void clientConfig::import_db(){
	
	/**查询库里已有的数据，并放入到一个set里，避免重复insert**/
	if(in_db_item.size() <= 0){
		string sql = "SELECT query FROM tb_dcg_query WHERE tid =? AND query IN (SELECT DISTINCT query FROM tb_dcg_data_list WHERE task_id=?)";
		vector<string> param;
		param.push_back(task_id);
		param.push_back(task_id);
		map<int,map<string,string> > ret;
		db->fetch_all(sql, param, ret);
		MD5 md5;
		map<int,map<string,string> >::iterator iter;
		map<string, string>::iterator iter_eng;
		for(iter=ret.begin(); iter!=ret.end(); iter++){
			string query = (iter->second)["query"];
			md5.reset();
			md5.update(query);
			string md5v = md5.toString();
			for(iter_eng=engine_info.begin(); iter_eng!=engine_info.end(); iter_eng++){
				string eid = iter_eng->first;
				string item_file = task_dir;
				item_file.append("/").append(task_id).append("_").append(eid).append("/").append(md5v).append(".item");
				in_db_item.insert(item_file);
			}
		}
	}
	
	/**遍历种子文件**/
	FILE* fp = fopen(seed_file.c_str(),"r");
	if(NULL == fp){
		perror("[clientConfig::import_db] open seed file failed");
		return;
	}
	char tmp[4096] = {0};
	while( !feof(fp) ){
		fgets(tmp,4096,fp);
		if( strlen(tmp) <= 0){
			continue;
		}
		string line(tmp);
		Utils::trim(line,"\t\n\r ");
		bzero(tmp,4096);
		if(line.find('\t') == string::npos){
			continue;
		}
		
		//切割，检查
		vector<string> items;
		Utils::explode(line,"\t",items);
		if( items.size() < 3){
			continue;
		}
		string local_file = items[1];
		string query = items[2];
		string item_file,engine_id;
		get_engine_item(local_file, engine_id, item_file);
		if(in_db_item.find(item_file) == in_db_item.end()){
			import_item_file(local_file, query);
		}
	}
	fclose(fp);
}



/**将一个item文件导入数据库**/
void clientConfig::import_item_file(const string& local_file, const string& query){
	string item_file,engine_id;
	get_engine_item(local_file, engine_id, item_file);
	if( access(item_file.c_str(),R_OK) != 0 ){
		return;
	}
	
	string item = "";
	FILE* fp = fopen(item_file.c_str(),"r");
	if(NULL == fp){
		perror("[clientConfig::import_item_file] open item file failed");
		return;
	}
	Json::Reader reader;
	Json::Value root;
	char tmp[2048] = {0};
	while( !feof(fp) ){
		fgets(tmp,2048,fp);
		if(strlen(tmp) <= 0){
			continue;
		}
		item.append(tmp);
		bzero(tmp,2048);
	}
	fclose(fp);
	if (!reader.parse(item.c_str(), root, false)){
        fprintf(stderr,"[clientConfig::import_item_file] parse json data failed,%s\n",item_file.c_str());
		return;
	}
	if(!root.isMember("errno") || !root.isMember("errmsg")){
		fprintf(stderr,"[clientConfig::import_item_file] errno/errmsg not exists,file[%s] json[%s]\n",item_file.c_str(),item.c_str());
		return;
	}
	if(root["errno"].asInt() != 0){
		fprintf(stderr,"[clientConfig::import_item_file] invalid errno,file[%s] json[%s]\n",item_file.c_str(),item.c_str());
		return;
	}
	if(!root["errmsg"].isMember("list")){
		fprintf(stderr,"[clientConfig::import_item_file] list not exists,file[%s] json[%s]\n",item_file.c_str(),item.c_str());
		return;
	}
	string sql = "DELETE FROM `tb_dcg_data_list` WHERE `task_id`=? AND `engine`=? AND `query`=?";
	vector<string> param;
	param.push_back(task_id);
	param.push_back(engine_id);
	param.push_back(query);
	db->execute(sql,param);
	string exec_sql = "INSERT INTO `tb_dcg_data_list` (`task_id`,`engine`,`query`,`rank`,`url`,`docid`,`title`,`realurl`) VALUES ";
	vector<string> exec_param;
	int size = (root["errmsg"]["list"]).size();
	int cnt = 0;
	for(;cnt < size; cnt++){
		string raw_url = root["errmsg"]["list"][cnt]["raw_url"].asString();
		string docid = root["errmsg"]["list"][cnt]["docid"].asString();
		string title = root["errmsg"]["list"][cnt]["title"].asString();
		string url = root["errmsg"]["list"][cnt]["url"].asString();
		string ad_type = root["errmsg"]["list"][cnt]["ad_type"].asString();
		string rank = root["errmsg"]["list"][cnt]["rank"].asString();
		exec_sql.append("(?,?,?,?,?,?,?,?),");
		exec_param.push_back(task_id);
		exec_param.push_back(engine_id);
		exec_param.push_back(query);
		exec_param.push_back(rank);
		exec_param.push_back(url.size() > 0 ? url : raw_url);
		exec_param.push_back(docid);
		exec_param.push_back(title);
		exec_param.push_back("");
	}
	Utils::rtrim(exec_sql,",");
	if( cnt > 0){
		db->execute(exec_sql,exec_param);
		in_db_item.insert(item_file);
	}
}


clientConfig::~clientConfig(){
	delete db;
}


/**将获取的配置信息打印出来**/
void clientConfig::printConf(){
	fprintf(stderr,"[clientConfig::printConf]***********config info********************\n");
	fprintf(stderr,"config file[%s]\n\n",cf.c_str());
	
	/**MySQL**/
	fprintf(stderr,"mysql_host[%s]\n",mysql_host.c_str());
	fprintf(stderr,"mysql_port[%s]\n",mysql_port.c_str());
	fprintf(stderr,"mysql_password[%s]\n",mysql_password.c_str());
	fprintf(stderr,"mysql_charset[%s]\n",mysql_charset.c_str());
	fprintf(stderr,"mysql_user[%s]\n",mysql_user.c_str());
	fprintf(stderr,"mysql_db[%s]\n\n",mysql_db.c_str());
	
	
	/**dir**/
	fprintf(stderr,"snapshot_dir[%s]\n",snapshot_dir.c_str());
	fprintf(stderr,"task_dir[%s]\n",task_dir.c_str());
	fprintf(stderr,"task_id[%s]\n",task_id.c_str());
	fprintf(stderr,"seed_file[%s]\n",seed_file.c_str());
	
	
	/**proxy api**/
	fprintf(stderr,"proxy_api[%s]\n",proxy_api.c_str());
	
	
	/**others**/
	fprintf(stderr,"dest_host[%s]\n",dest_host.c_str());
	fprintf(stderr,"page_num_per_proxy[%s]\n",page_num_per_proxy.c_str());
	fprintf(stderr,"normal_page_limit_size[%s]\n",normal_page_limit_size.c_str());
	fprintf(stderr,"page_counter[%d]\n",page_counter);

	
	/**host list**/
	vector<string>::iterator iter;
	for(iter=download_hosts.begin(); iter!=download_hosts.end(); iter++){
		fprintf(stderr,"download_hosts[%s]\n",(*iter).c_str());
	}
	for(iter=parse_hosts.begin(); iter!=parse_hosts.end(); iter++){
		fprintf(stderr,"parse_hosts[%s]\n",(*iter).c_str());
	}
	for(iter=sogou_parse_hosts.begin(); iter!=sogou_parse_hosts.end(); iter++){
		fprintf(stderr,"sogou_parse_hosts[%s]\n",(*iter).c_str());
	}

	/**engine info**/
	map<string,string>::iterator iter_eng;
	for(iter_eng=engine_info.begin(); iter_eng!=engine_info.end(); iter_eng++){
		fprintf(stderr,"engine_id[%s]\tengine_name[%s]\n",(iter_eng->first).c_str(),(iter_eng->second).c_str());
	}
	
	/**proxy host**/
	for(iter=proxy_list.begin(); iter!=proxy_list.end(); iter++){
		fprintf(stderr,"proxy_host[%s]\n",(*iter).c_str());
	}
}
