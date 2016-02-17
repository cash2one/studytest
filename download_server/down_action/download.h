#ifndef _PAGEDOWNLOAD_H_
#define _PAGEDOWNLOAD_H_
#include "downConfig.h"

/**
 * 网页下载控制类
 */
class download{

public:
	~download();
	
	
	download( downConfig* cnf, const std::string& proxy);
	
	
	/**开始下载：就这么多url list，下完就退出**/
	int32_t run( const std::vector<std::string>& down_list );

private:

	/**代理的ip:port**/
	std::string haproxy;
	
	
	/**下载配置类**/
	downConfig* conf;
	
	
	/**CURL的一些句柄**/
	CURL* curl_handle;

	
	
	/**init something**/
	int32_t init_something();
	
	
	/**下载一个：line的格式为：url  \t  location **/
	int32_t download_one_page(const std::string& line);
};


#endif

