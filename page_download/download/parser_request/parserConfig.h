#ifndef _PARSERCONFIG_H_
#define _PARSERCONFIG_H_
#include<string>
#include<vector>
#include<map>
#include<stdint.h>

using namespace std;

class parserConfig{

public:
	parserConfig(const string& conf_file, const string& seed_file);
	
	//根据html拼凑item file
	int32_t get_item_file(const string& html_file, string& item_file);
	
	//从html_file里提取引擎编号
	int32_t get_engine_id(const string& html_file, string& engine);

    void print_conf();
	
	//任务文件目录
	string task_dir;
	
	//快照目录
	string local_snapshot_dir;
	string remote_snapshot_dir;
	
	//host目录
	vector<string> hosts;
	
	//任务编号
	uint32_t task_id;

	//引擎列表 1:sogou, 2:baidu 这样的
	map<string,string> engine_info;
	
	//是否要广告，0不要，1要（包括品牌广告和顶部一般广告）
	int32_t need_ad;
	
	//mysql账号信息
	string mysql_host;
	string mysql_port;
	string mysql_password;
	string mysql_charset;
	string mysql_user;
	string mysql_db;
	
	//页面文件大小限制
	uint32_t page_limit_size;
	
	//当前客户端所在机器的IP地址，回传用的
	string item_dest_host;
	string html_dest_host;
	
	//要读取的种子文件各字段的分隔符及html文件是第几个字段（从1开始的哟）
	uint32_t html_file_index;
	
	//下载程序地址
	string download;
};
#endif

