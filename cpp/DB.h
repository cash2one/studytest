#ifndef _DB_H_
#define _DB_H_

#include <mysql/mysql.h>
#include <map>
#include <vector>
#include <string>
#include "Utils.h"

using namespace std;

class DB{
public:
	DB();
	~DB();
	int initDB();
	int query(const string& sql, const vector<string>& param);
	int execute(const string& sql, const vector<string>& param);
	int fetch_one(const string& sql, const vector<string>& param, string& ret);
	int fetch_all(const string& sql, const vector<string>& param, map<int,map<string,string>>& ret);
	int fetch_row(const string& sql, const vector<string>& param, map<string,string>& ret);
	int update(const string& table, const map<string,string>& update_data, const map<string,vector<string>>& where);
	int insert(const string& table, const map<string,string>& data);
	int insert_update(const string& table, const map<string,string>& insert_data, const map<string,string>& update_data);
	
	///设置账号信息 
	void set_connect(string h, string u, string d, string p, string c, int pt);
	
protected:
	MYSQL *mysql;
	MYSQL_STMT *stmt;
	static map<string,string> paramMap;
	
	//mysql连接信息
	string mysql_host;
	string mysql_user;
	string mysql_db;
	string mysql_passwd;
	int port;
	string charset;
	
	///内部辅助方法
	int format_where_cond(const map<string,vector<string>>& param, string& where, vector<string>& data);
	int format_sql(const map<string,string>& param, string& sql, vector<string>& data);
	int bind_param_execute(const string& sql, const vector<string>& param);
	int get_execute_result(map<int,map<string,string>>& ret);
};

#endif

