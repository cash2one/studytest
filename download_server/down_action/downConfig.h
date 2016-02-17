#ifndef __DOWN_CONFIG_H__
#define __DOWN_CONFIG_H__
#include <string>
#include <list>
#include <vector>
#include<curl/curl.h>
#include <stdint.h>


class downConfig{

public:
	~downConfig();
	downConfig(const std::string& conf_file);
	
	
	/**处理下载好的页面用的**/
    std::string sendfile_shell;
	
	
	/**本地临时的html目录**/
	std::string tmp_html_dir;
	
	
	/**headers列表**/
	std::list<std::string> header_list;
	
	
	/**header共用的**/
	curl_slist *headers;
	
	
	/**也是各线程间可以共用的东西**/
	CURLSH* share_handle;
	
	
	/**下载的超时timeout**/
	int32_t timeout;
	
	
private:

	std::string cf;
	
	void init_env();
};

#endif

