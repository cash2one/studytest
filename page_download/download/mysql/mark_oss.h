#ifndef _MARK_OSS_H_
#define _MARK_OSS_H_
#include "utils/Utils.h"
#include "mysql/DB.h"


class mark_oss: public DB{
public:
	mark_oss();
	mark_oss(string h, string u, string d, string p, string c, int pt);
	int getTotal(const string& table, const map<string,vector<string> >& where);
	int getList(const string& table, const map<string,vector<string> >& where, int page, int pagesize,
				const string& format_field, const map<string, string>& orderby, map<string, map<string, string> >& ret );
	///不需要orderby、format_field
	int getList(const string& table, const map<string,vector<string> >& where, int start, int limit, map<int, map<string, string> >& ret );
	int get_task_info(int task_id, map<string,string>& ret);
	
private:
	int format_orderby_limit(int page, int pagesize, const map<string, string>& orderby, int limit_type, string& ret);
};
#endif


