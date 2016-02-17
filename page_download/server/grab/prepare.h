#ifndef _PREPARE_H_
#define _PREPARE_H_

#include "mysql/mark_oss.h"

using namespace std;

class prepare{
public:
	prepare(int t, int s, int l):task_id(t),start(s), limit(l) {}
	int run(); ///开始运行
	
private:
	map<string, string> task_info;
	map<string, string> engine_local_file;
	mark_oss db;
	int task_id;
	int start;
	int limit;
	

	int check_env();///初始化各种环境
	int generate_seed(); ///产生种子文件
	int get_eid_list(const string& eid_str, vector<string>& eids);
};

#endif

