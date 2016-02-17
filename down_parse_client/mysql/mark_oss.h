#ifndef _MARK_OSS_H_
#define _MARK_OSS_H_
#include "DB.h"


class mark_oss: public DB{
public:
	mark_oss();
	mark_oss(std::string h, std::string u, std::string d, std::string p, std::string c, int pt);
	int getTotal(const std::string& table, const std::map<std::string,std::vector<std::string> >& where);
	int getList(const std::string& table, const std::map<std::string,std::vector<std::string> >& where, int page, int pagesize,
				const std::string& format_field, const std::map<std::string, std::string>& orderby, std::map<std::string, std::map<std::string, std::string> >& ret );
	///不需要orderby、format_field
	int getList(const std::string& table, const std::map<std::string,std::vector<std::string> >& where, int start, int limit, std::map<int, std::map<std::string, std::string> >& ret );
	int get_task_info(int task_id, std::map<std::string,std::string>& ret);
	
private:
	int format_orderby_limit(int page, int pagesize, const std::map<std::string, std::string>& orderby, int limit_type, std::string& ret);
};
#endif


