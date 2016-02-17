#ifndef _MARK_OSS_H_
#define _MARK_OSS_H_

#include "DB.h"


class mark_oss: public DB{
public:
	mark_oss();
	int getTotal(const string& table, const map<string,vector<string>>& where);
	int getList(const string& table, const map<string,vector<string>>& where, int page, int pagesize,
			const string& format_field, const map<string, string>& orderby, map<string, map<string, string>>& ret );

};
#endif


