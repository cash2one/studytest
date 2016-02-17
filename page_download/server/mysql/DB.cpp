#include "mysql/DB.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

map<string,string> DB::paramMap;

DB::DB():mysql(NULL),stmt(NULL){
	paramMap.insert(make_pair("lt","<"));
	paramMap.insert(make_pair("gt",">"));
	paramMap.insert(make_pair("eq","="));
	paramMap.insert(make_pair("neq","!="));
	paramMap.insert(make_pair("lte","<="));
	paramMap.insert(make_pair("gte",">="));
	paramMap.insert(make_pair("in","IN"));
	paramMap.insert(make_pair("llike","LIKE"));
	paramMap.insert(make_pair("rlike","LIKE"));
	paramMap.insert(make_pair("like","LIKE"));
	paramMap.insert(make_pair("find","FIND_IN_SET"));
}

/**
 *析构，释放掉各种资源
 */
DB::~DB(){
	if(NULL != stmt){
		 mysql_stmt_close(stmt);
	}
	if(NULL != mysql){
		mysql_close(mysql);
	}
}

/**
 * 设置连接账号信息
 */
void DB::set_connect(string h, string u, string d, string p, string c, int pt){
	charset = c;
	port = pt;
	mysql_host = h;
	mysql_passwd = p;
	mysql_user = u;
	mysql_db = d;
}

/**
 *初始化数据库连接
 */
int DB::initDB(){
	if(NULL == mysql){
		mysql = mysql_init(NULL);
		if(!mysql_real_connect(mysql, mysql_host.c_str(), mysql_user.c_str(),
			mysql_passwd.c_str(), mysql_db.c_str(), port, NULL, 0)){
			fprintf(stderr,"connect MySQL failed:%s\n",mysql_error(mysql));
			exit(EXIT_FAILURE);
		}
		string init_sql = "SET NAMES ";
		init_sql += charset;
		mysql_real_query(mysql,  init_sql.c_str(), init_sql.length());
	}
	if(NULL == stmt){
		stmt = mysql_stmt_init(mysql);
		if(!stmt){
			fprintf(stderr,"mysql_stmt_init failed");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}

/**
 *执行一条SQL语句，通用的
 */
int DB::query(const string& sql, const vector<string>& param){
	initDB();
	return bind_param_execute(sql, param);
}

/**
 *执行一条SQL语句，返回影响的行数
 * 主要用于不需要返回结果的如update、delete等。
 */
int DB::execute(const string& sql, const vector<string>& param){
	int ret = query(sql, param);
	if(0 == ret){
		return (int)mysql_stmt_affected_rows(stmt);
	}
	return ret;
}

/**
 *获取一个字段的值，一般是第一个字段
 */
int DB::fetch_one(const string& sql, const vector<string>& param, string& result){
	int ret_status = query(sql, param);
	if( 0 != ret_status){
		return -1;
	}
	
	///获取结果
	map<int,map<string,string> > ret;
	get_execute_result(ret);
	if(ret.size() <= 0){
		return -2;
	}
	map<int,map<string,string> >::iterator iter;
	iter = ret.find(0);
	if( iter == ret.end() ){
		return -3;
	}
	map<string,string>::iterator iter1;
	iter1 = ret[0].begin();
	result = iter1->second;
	return 0;
}

/**
 * 获取所有结果
 */
int DB::fetch_all(const string& sql, const vector<string>& param, map<int,map<string,string> >& ret){
	int ret_status = query(sql, param);
	if( 0 != ret_status){
		return -1;
	}
	
	return get_execute_result(ret);
}

/**
 * 获取一行数据
 */
int DB::fetch_row(const string& sql, const vector<string>& param, map<string,string>& result){
	int ret_status = query(sql, param);
	if( 0 != ret_status){
		return -1;
	}
	
	///获取结果
	map<int,map<string,string> > ret;
	get_execute_result(ret);
	if(ret.size() <= 0){
		return -2;
	}
	map<int,map<string,string> >::iterator iter;
	iter = ret.find(0);
	if( iter == ret.end() ){
		return -3;
	}
	result = ret[0];
	return 0;
}

/**
 * 执行一条UPDATE语句，返回影响行数
 */
int DB::update(const string& table, const map<string,string>& update_data, const map<string,vector<string> >& where){
	///要更新的字段
	vector<string> u_data;
	string u_sql;
	format_sql(update_data, u_sql, u_data);
	
	///where条件
	string w_sql;
	vector<string> w_data;
	format_where_cond(where, w_sql, w_data);
	if( w_sql.length() > 0){
		w_sql.insert(0," WHERE ");
	}
	
	//汇总所有的语句和数据
	w_data.insert(w_data.begin(),u_data.begin(),u_data.end());
	string sql = "UPDATE `";
	sql.append(table).append("` SET ").append(u_sql).append(w_sql);

	return execute(sql, w_data);
}

/**
 *执行一条插入语句，返回last_insert_id
 */
int DB::insert(const string& table, const map<string,string>& data){
	string i_sql;
	vector<string> i_data;
	format_sql(data, i_sql, i_data);
	
	string sql = "INSERT INTO `";
	sql.append(table).append("` SET ").append(i_sql);
	int ret = execute(sql, i_data);
	if(ret > 0){
		return (int)mysql_stmt_insert_id(stmt);
	}
	return ret;
}

/**
 *执行一条INSERT INTO ......  ON DUPLICATE KEY UPDATE ........语句，即，先插入，若唯一的健冲突则执行更新操作
 ，返回影响行数
 */
int DB::insert_update(const string& table, const map<string,string>& insert_data, const map<string,string>& update_data){
	if(insert_data.size() <= 0 || update_data.size() <= 0){
		return -1;
	}
	
	///要插入的数据
	string i_sql;
	vector<string> i_data;
	format_sql(insert_data, i_sql, i_data);
	
	//要更新的数据
	string u_sql;
	vector<string> u_data;
	format_sql(update_data, u_sql, u_data);
	
	///汇总最终的数据
	u_data.insert(u_data.begin(),i_data.begin(),i_data.end());
	string sql = "INSERT INTO `";
	sql.append(table).append("` SET ").append(" ON DUPLICATE KEY UPDATE ").append(u_sql);

	return execute(sql, u_data);
}

/**
 * 组装SQL，就是将a=>2  b=>3这样的map组装成：a=?,b=?，并且将数据返回
 */
int DB::format_sql(const map<string,string>& param, string& sql, vector<string>& data){
	if(param.size() <= 0){
		return -1;
	}
	sql.clear();
	data.clear();
	
	///开始遍历字段
	map<string,string>::const_iterator iter;
	for(iter = param.begin(); iter != param.end(); iter++){
		sql.append("`").append(iter->first).append("`=?,");
		data.push_back(iter->second);
	}
	Utils::trim(sql,',');
	return 0;
}

/**
 * 拼接where条件，只支持and关系，param = {
 *			"id:eq"=>3,  // id = 3-----vector
 *			"name:like"=>'wendao' // name like %wendao%------vector
			'status:in'=>(1,2), // status IN (1,2)  -----vector
			'list:find'=>'liu',  // FIND_IN_SET('liu',`list`)------vector
 *}
 * 填充后的值如下：
 * where ：”a= ? and b = ?“
 * data = 1, 2,.....
 */
int DB::format_where_cond(const map<string,vector<string> >& param, string& where, vector<string>& data){
	if(param.size() <= 0){
		return -1;
	}
	where.clear();
	
	///遍历param
	map<string,vector<string> >::const_iterator iter;
	map<string,string>::const_iterator iter1;
	vector<string>::iterator vec_iter;
	for(iter = param.begin(); iter != param.end(); iter++){
		//key和value
		string key = iter->first;
		vector<string> value = iter->second;
		if(value.size() <= 0){
			continue;
		}
		
		string sql_compare_symbol = "="; //如> <
		string compare_symbol = "="; //如gt lt
		int pos = key.find(":");
		///key里是否含有：
		if( pos > 0){
			compare_symbol = key.substr(pos+1, key.length()-pos-1);
			key = key.substr(0,pos);
			iter1 = paramMap.find(compare_symbol);
			if(iter1 != paramMap.end()){
				sql_compare_symbol = paramMap[compare_symbol];
			}
		}
		
		///依次判断
		if( sql_compare_symbol.compare("IN") == 0){ ///in
			where.append("`").append(key).append("` IN (");
			for(vec_iter = value.begin(); vec_iter != value.end(); vec_iter++){
				where.append("?,");
				data.push_back(*vec_iter);
			}
			Utils::trim(where,',');
			where.append(") AND ");
		}
		else if(sql_compare_symbol.compare("LIKE") == 0){ //like
			if(compare_symbol.compare("llike") == 0){
				where.append("`").append(key).append("` LIKE '?%' AND ");
				data.push_back(value[0]);
			}
			else if(compare_symbol.compare("like") == 0){
				where.append("`").append(key).append("` LIKE '%?%' AND ");
				data.push_back(value[0]);
			}
			else if(compare_symbol.compare("rlike") == 0){
				where.append("`").append(key).append("` LIKE '%?' AND ");
				data.push_back(value[0]);
			}
		}
		else if(sql_compare_symbol.compare("FIND_IN_SET") == 0){
			where.append(" FIND_IN_SET(?,`").append(key).append("`) AND ");
			data.push_back(value[0]);
		}
		else{
			where.append("`").append(key).append("`").append(sql_compare_symbol).append("? AND ");
			data.push_back(value[0]);
		}
	}
	
	Utils::rtrim(where,"AND ");
	return 0;
}

/**
 *绑定SQL与参数，开始预处理
 */
int DB::bind_param_execute(const string& sql, const vector<string>& param){
	if(sql.length() <= 0 ){
		return -1;
	}
	
	///预处理
    if(mysql_stmt_prepare(stmt,sql.c_str(),strlen(sql.c_str()) ) !=0){
        fprintf(stderr,"prepare sql failed:%s\n",mysql_stmt_error(stmt));
        return -1;
    }
	
	///要绑定的参数个数
	unsigned long para_num = mysql_stmt_param_count(stmt);
	if(para_num != param.size()){
		fprintf(stderr,"bind param failed\n");
		return -2;
	}
	
	///需要绑定参数的bind结构体
	int bind_size = sizeof(MYSQL_BIND)*para_num;
    MYSQL_BIND *bind = (MYSQL_BIND*)malloc(bind_size);
    bzero(bind, bind_size);
	
	///各个参数的长度
	int len_size = sizeof(long unsigned int *)*para_num;
	long unsigned int *param_len = (long unsigned int *)malloc(len_size);
	bzero(param_len,len_size);
	
	///设置各个参数的属性
	for(vector<string>::size_type i=0; i < param.size(); i++){
		param_len[i] = param[i].length();
		bind[i].buffer_type = MYSQL_TYPE_STRING;
		bind[i].buffer = (void*)param[i].c_str();
		bind[i].is_null = 0;
		bind[i].length = &(param_len[i]);
	}
	
	///绑定参数并执行
    if(mysql_stmt_bind_param(stmt,bind) != 0){
        fprintf(stderr,"mysql_stmt_bind_param failed:%s\n",mysql_stmt_error(stmt));
		free(bind);
		free(param_len);
        return -2;
    }
    if(mysql_stmt_execute(stmt)!=0){
        fprintf(stderr,"execute failed:%s\n",mysql_stmt_error(stmt));
		free(bind);
		free(param_len);
        return -2;
    }
	free(bind);
	free(param_len);
	return 0;
}

/**
 *获取执行的结果
 */
int DB::get_execute_result(map<int,map<string,string> >& ret){
	ret.clear();
	
	///返回结果的列数
    MYSQL_RES* meta_data = mysql_stmt_result_metadata(stmt);
    int column_num = mysql_num_fields(meta_data);
	
	///要绑定的结果
	int bind_size = sizeof(MYSQL_BIND)*column_num;
    MYSQL_BIND *bind = (MYSQL_BIND*)malloc(bind_size);
    bzero(bind,bind_size);
	
	//各字段实际长度
	int ret_len_size = sizeof(long unsigned int *)*column_num;
	long unsigned int *ret_len  = (long unsigned int *)malloc(ret_len_size);
	bzero(ret_len, ret_len_size);
	
	//确定每个字段的空间
	int max_buffer_size = 10240;
	for(int i=0; i < column_num; i++){
		int buffer_size = (meta_data->fields[i].length > max_buffer_size) ? max_buffer_size : meta_data->fields[i].length;
		(bind + i)->buffer = malloc(buffer_size + 1);
		(bind + i)->buffer_length = buffer_size + 1;
		(bind + i)->length = &(ret_len[i]);
	}
	
	///获取查询的结果
    if(mysql_stmt_bind_result(stmt, bind) != 0){
        fprintf(stderr,"mysql_Stmt_bind_result failed: %s\n",mysql_stmt_error(stmt));
        exit(EXIT_FAILURE);
    }
    if(0 != mysql_stmt_store_result(stmt)){
        fprintf(stderr,"store_result failed: %s\n",mysql_stmt_error(stmt));
        exit(EXIT_FAILURE);
    }
	

	///将结果存入map里
	int row_num = 0;
	while(!mysql_stmt_fetch(stmt)){
		unsigned long  offset_len = 0;
		for(int j=0; j<column_num; j++){
			if(mysql_stmt_fetch_column(stmt,bind,j,offset_len) != 0){
				fprintf(stderr, "mysql_stmt_fetch_column failed\n");
				continue;
			}
			ret[row_num].insert(make_pair(meta_data->fields[j].name,(char*)(bind + j)->buffer));
			offset_len += ret_len[j];
		}
		row_num++;
	}
	
	///清理现场
	mysql_stmt_free_result(stmt);
	for(int i=0; i < column_num; i++){
		free((bind + i)->buffer);
	}
	free(bind);
	free(ret_len);
	
	return 0;
}


/**
DB db;
db.initDB();
///测试一个字段
string sql = "select name from tb_engine where eid=?";
vector<string> param;
param.push_back("42");
string result;
db.fetch_one(sql,param,result);
cout<<result<<endl;

///测试一条数据
string sql1 = "select * from tb_engine where eid=?";
vector<string> param1;
param1.push_back("42");
map<string,string> result1;
db.fetch_row(sql1,param1,result1);
map<string,string>::iterator iter;
cout<<endl;
cout<<endl;
for(iter = result1.begin(); iter != result1.end(); iter++){
	cout<<iter->first<<"\t"<<iter->second<<endl;
}

///测试返回所有结果的
string sql2 = "select * from experiment_detail";
vector<string> param2;
map<int,map<string,string> > result2;
db.fetch_all(sql2,param2,result2);
map<int,map<string,string> >::iterator iter2;
map<string,string>::iterator iter21;
cout<<endl;
cout<<endl;
for(iter2=result2.begin(); iter2 != result2.end(); iter2++){
	cout<<"================row: "<<iter2->first<<endl;
	for(iter21=iter2->second.begin(); iter21!=iter2->second.end(); iter21++){
		cout<<"field="<<iter21->first<<"\tvalue="<<iter21->second<<endl;
	}
}

///测试一条INSERT语句
map<string,string> param_insert;
param_insert["exp_id"] = "8888";
param_insert["module_name"] = "10";
param_insert["para_name"] = "测试一条数据";
param_insert["para_value"] = "这是直接插入的";
param_insert["para_desc"] = "描述信息";
cout<<endl;
cout<<endl;
int last_insert_id = db.insert("experiment_detail_para", param_insert);
cout<<"last_insert_id="<<last_insert_id<<endl;

///测试一条UPDATE语句
map<string,string> update_data;
update_data["para_desc"] = "ABCEDDDDDD";
vector<string> update_arg;
update_arg.push_back("100");
update_arg.push_back("101");
map<string,vector<string> > update_where;
update_where["detail_para_id:in"] = update_arg;
db.update("experiment_detail_para",update_data,update_where);

///第二条UPDATE语句
map<string,string> update_data2;
update_data2["para_desc"] = "ASDFASDFSADFASD";
vector<string> update_arg2;
update_arg2.push_back("100");
map<string,vector<string> > update_where2;
update_where2["detail_para_id:gt"] = update_arg2;
db.update("experiment_detail_para",update_data2,update_where2);
return 0;
 */


