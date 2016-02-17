#include "mark_oss.h"


mark_oss::mark_oss():DB() {
	charset = "UTF8";
	port = 3306;
	mysql_host = "localhost";
	mysql_user = "phpmyadmin";
	mysql_passwd = "123456";
	mysql_db = "mark_oss";
}

/**
 * 获取总数
 */
int mark_oss::getTotal(const string& table, const map<string,vector<string> >& where){
	///拼接where部分
	string where_sql;
	vector<string> w_data;
	format_where_cond(where, where_sql, w_data);
	if(where_sql.length() > 0){
		where_sql.insert(0, " WHERE ");
	}

	
	///组装最终的SQL语句
	string sql = "SELECT COUNT(*) FROM `";
	sql.append(table).append("`").append(where_sql);
	
	//查询返回结果
	string result;
	fetch_one(sql, w_data, result);
	return atoi(result.c_str());
}

/**
 * 获取结果列表，并且可以按照某个字段格式化返回的结果
 *
 * @param	table			数据表名
 * @param	where			要查询的WHERE条件
 * @param	page			页码，从1开始
 * @param	pagesize		页大小，最小为1
 * @param	format_field	用来格式化返回结果的字段，此字段的值将作为结果map的first，而且它对应的记录则为second，必须保证唯一性 
 * @param	orderby			用来生成ORDER BY语句的map，second一般为"ASC"、"DESC"
 * @param	ret 			用来存放返回结果的map，其first即是上面提到的用来格式化的字段format_field
 *
 * @return	ret				查询无误返回0，出错时则返回负数
 */
int mark_oss::getList(const string& table, const map<string,vector<string> >& where, int page, int pagesize, 
						const string& format_field, const map<string, string>& orderby, map<string, map<string, string> >& ret )
{
	string orderby_limit;
	format_orderby_limit(page, pagesize, orderby, 1, orderby_limit);
	
	///拼接where部分
	string where_sql;
	vector<string> w_data;
	format_where_cond(where, where_sql, w_data);
	if(where_sql.length() > 0){
		where_sql.insert(0, " WHERE ");
	}
	
	
	///最终，组装成完整的SQL语句
	string sql = "SELECT * FROM `";
	sql.append(table).append("` ").append(where_sql).append(orderby_limit);
	
	
	///执行查询，并格式化结果
	map<int, map<string,string> > q_ret;
	fetch_all(sql, w_data, q_ret);
	
	
	///判断是否需要格式化
	map<int, map<string,string> >::iterator outer;
	map<string,string>::iterator inner;
	for(outer=q_ret.begin(); outer!=q_ret.end(); outer++){
		inner = outer->second.find(format_field);
		string key = "";
		if(inner == outer->second.end()){ ///结果里，没有此字段，还保持原来的格式
			char tmp[10] = {0};
			sprintf(tmp,"%d",outer->first);
			key = tmp;
		}
		else{
			key = outer->second[format_field];
		}
		ret[key] = outer->second;
	}
	
	return 0;
}

/**
 * 上面getList的简化版，没有format_field、orderby等，并且limit变成了直接LIMIT start, limit这样的查询规则
 *
 * @param	table			数据表名
 * @param	where			要查询的WHERE条件
 * @param	start			从哪条记录开始查询
 * @param	limit			要获取的记录条数
 * @param	ret 			用来存放返回结果的map
 *
 * @return	ret				查询无误返回0，出错时则返回负数
 *
 */
int mark_oss::getList(const string& table, const map<string,vector<string> >& where, int start, int limit, map<int, map<string, string> >& ret ){
	string orderby_limit;
	map<string, string> orderby;
	format_orderby_limit(start, limit, orderby, 0, orderby_limit);
	
	///拼接where部分
	string where_sql;
	vector<string> w_data;
	format_where_cond(where, where_sql, w_data);
	if(where_sql.length() > 0){
		where_sql.insert(0, " WHERE ");
	}
	
	
	///最终，组装成完整的SQL语句
	string sql = "SELECT * FROM `";
	sql.append(table).append("` ").append(where_sql).append(orderby_limit);
	
	
	///执行查询
	fetch_all(sql, w_data, ret);
	
	return 0;
}

/**
 *获取单条任务信息
 *
 * @param	int		task_id		任务编号
 * @param	map		ret			该任务的详细信息
 */
int mark_oss::get_task_info(int task_id, map<string,string>& ret){
	if(task_id <= 0){
		return -1;
	}
	ret.clear();
	
	///开始查询
	string sql = "SELECT * FROM `tb_dcg_task` WHERE `tid`=?";
	vector<string> param;
	char task_id_str[10] = {0};
	sprintf(task_id_str, "%d", task_id);
	param.push_back(task_id_str);
	return fetch_row(sql,param, ret);
}

/**
 * 组装 ORDER BY ..... LIMIT xx,xx 部分
 * limit_type：0表示直接limit page,pagesize，1表示 limit (page-1)*pagesize, pagesize
 */
int mark_oss::format_orderby_limit(int page, int pagesize, const map<string, string>& orderby, int limit_type, string& ret){
	///先处理 ORDER BY语句
	string order = "";
	if(orderby.size() > 0){
		order.append(" ORDER BY ");
		map<string, string>::const_iterator iter;
		for(iter=orderby.begin(); iter != orderby.end(); iter++){
			order.append(iter->first).append(" ").append(iter->second).append(",");
		}
		Utils::trim(order, ',');
	}
	
	
	///再处理LIMIT部分
	pagesize = (pagesize <= 0) ? 1 : pagesize;
	int start;
	if(1 == limit_type){
		page = (page <= 0) ? 1 : page;
		start = ( page - 1 ) * pagesize;
	}
	else{
		start = page;
	}
	char start_str[10] = {0};
	char pagesize_str[10] = {0};
	sprintf(start_str,"%d",start);
	sprintf(pagesize_str,"%d",pagesize);
	string limit = " LIMIT ";
	limit.append(start_str).append(",").append(pagesize_str);
	
	//最终的结果
	ret = order;
	ret.append(limit);
	return 0;
}
