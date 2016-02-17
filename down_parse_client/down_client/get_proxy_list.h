#ifndef _GET_PROXY_LIST_H_
#define _GET_PROXY_LIST_H_
#include<list>
#include<vector>
#include "clientConfig.h"

class get_proxy_list{

public:
	get_proxy_list(int n, clientConfig* config);

	/**发起请求**/
	int request(std::vector<std::string>& ret);
	
private:
	clientConfig* conf;
	int num;
	
	/**要请求api的URL**/
    std::string api_url;

    std::string url;
};
#endif



