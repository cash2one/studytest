#ifndef _DB_H_
#define _DB_H_
#include "utils/Utils.h"
#include <mysql.h>
#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>



class DB{
public:
	DB();
	~DB();
	int initDB();
	
	
	//执行一条SQL，返回affected_rows
	int execute(const std::string& sql, const std::vector<std::string>& param);
	
	
	//获取一个字段的值，一般是第一个字段了
	int fetch_one(const std::string& sql, const std::vector<std::string>& param, std::string& ret);
	
	
	//返回SQL语句所有的结果
	int fetch_all(const std::string& sql, const std::vector<std::string>& param, std::map<int,std::map<std::string,std::string> >& ret);
	
	
	//返回一行结果
	int fetch_row(const std::string& sql, const std::vector<std::string>& param, std::map<std::string,std::string>& ret);
	
	
	//执行一条UPDATE语句，返回affected_rows
	int update(const std::string& table, const std::map<std::string,std::string>& update_data, const std::map<std::string,std::vector<std::string> >& where);
	
	
	//执行一条INSERT语句，返回affected_rows
	int insert(const std::string& table, const std::map<std::string,std::string>& data);
	
	
	//执行INSRTO INTO .... ON DUPLICATE KEY UPDATE.......返回affected_rows
	int insert_update(const std::string& table, const std::map<std::string,std::string>& insert_data, const std::map<std::string,std::string>& update_data);
	
	
	//设置账号信息 
	void set_connect(std::string h,std::string u,std::string d,std::string p,std::string c,int pt);
	
	
protected:
	MYSQL *mysql;
	MYSQL_STMT *stmt;
	static std::map<std::string,std::string> paramMap;
	
	//mysql连接信息
	std::string mysql_host;
	std::string mysql_user;
	std::string mysql_db;
	std::string mysql_passwd;
	int port;
	std::string charset;
	
	//内部辅助方法：拼凑where条件
	int format_where_cond(const std::map<std::string,std::vector<std::string> >& param, std::string& where, std::vector<std::string>& data);
	
	
	//组装SQL
	int format_sql(const std::map<std::string,std::string>& param, std::string& sql, std::vector<std::string>& data);
	
	
	//绑定SQL与参数，开始预处理
	int bind_param_execute(const std::string& sql, const std::vector<std::string>& param);
	
	
	//获取执行的结果
	int get_execute_result(std::map<int,std::map<std::string,std::string> >& ret);
	
	
	//通用的查询方法
	int query(const std::string& sql, const std::vector<std::string>& param);
};

#endif

