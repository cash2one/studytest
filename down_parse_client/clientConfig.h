#ifndef __CLIENT_CONFIG_H__
#define __CLIENT_CONFIG_H__
#include "mysql/mark_oss.h"
#include <list>
#include <set>
#include <stdint.h>
#include <pthread.h>



class clientConfig{

public:
	clientConfig(const std::string& conf_file, const std::string& tid);
	~clientConfig();
	
	
	
	/**从html file的地址里提取：engine_id、item_file**/
	void get_engine_item(const std::string& html_file, std::string& engine_id, std::string& item_file);
	
	
	
	/**获取engine info及生成种子文件**/
	void generateInfo();
	
	
	
	/**更改任务状态为抓取中或者抓取完成**/
	void set_task_status(const std::string& status);
	
	
	
	/**解析好的json数据入库**/
	void import_db();
	void import_item_file(const std::string& local_file, const std::string& query);
	
	
	/**MySQL连接句柄**/
	mark_oss* db;
	
	
	/**正式数据的MySQL账号信息**/
	std::string mysql_host;
	std::string mysql_port;
	std::string mysql_password;
	std::string mysql_charset;
	std::string mysql_user;
	std::string mysql_db;

	
	
	/**快照目录**/
	std::string snapshot_dir;
	
	
	
	/**任务目录**/
	std::string task_dir;
	
	
	
	/**下载服务端的地址列表**/
	std::vector<std::string> download_hosts;
	
	
	
	/**解析服务端地址列表**/
	std::vector<std::string> parse_hosts;
	
	
	
	/**解析sogou页面专用**/
	std::vector<std::string> sogou_parse_hosts;
	
	
	
	/**用于获取proxy的API地址**/
	std::string proxy_api;
	
	
	
	/**任务编号**/
	std::string task_id;
	
	
	
	/**任务的种子文件：url  \t   location   \t   query**/
	std::string seed_file;
	
	
	
	/**要下载的页面数量**/
	uint32_t page_counter;
	
	
	
	/**本任务所用的引擎信息：eid=>ename**/
	std::map<std::string,std::string> engine_info;

	
	
	/**页面大小限制**/
	std::string normal_page_limit_size;
	
	
	
	/**每个代理要下载的页面数量**/
	std::string page_num_per_proxy;
	
	
	
	/**回传处理好的文件的目标地址**/
	std::string dest_host;
	
	
	
	/**解析线程存活标志，下载线程要用**/
	int parse_thread_exit;
	
	
	
	/**代理IP列表**/
	std::vector<std::string> proxy_list;
	
	
	
	void update_proxy_list();
	
	
	
	/**已经入库的item**/
	std::set<std::string> in_db_item;
	
	
	
	/**是否是重试抓取**/
	int32_t is_retry;

	
	/**检查解析失败的页面，重新解析用的**/
	void check_parse_fail();

	
	
	/**打印获取到的配置信息**/
	void printConf();
private:

	/**配置文件**/
	std::string cf;
	
	
	/**解析配置中的列表部分**/
	void parseList(std::vector<std::string>& tl, const std::string& value);
};

#endif

