#include "get_proxy_list.h"
#include<curl/curl.h>
#include "utils/Utils.h"

using namespace std;
static char errorBuffer[CURL_ERROR_SIZE];


/**请求回调函数**/
static size_t write_data(char *data, size_t size, size_t nmemb, void *stream){
	string* writerData = (string*)stream;
    unsigned long sizes = size * nmemb;
    if (writerData == NULL){
		return 0;
	}
    writerData->append(data, sizes);
    return sizes;
}


get_proxy_list::get_proxy_list(int n, clientConfig* config):num(n),conf(config),url(""){
	char tmp[10] = {0};
	sprintf(tmp,"%d",num);
	url.append(conf->proxy_api).append("&num=").append(tmp);
}

int get_proxy_list::request(vector<string>& ret){
	string proxy_ret = "";
	
	
	CURL* curl_handle;
    curl_handle = curl_easy_init();
    if(!curl_handle){
        fprintf(stderr,"[get_proxy_list::request] curl_easy_init failed\n");
        return -1;
    }
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 3L);
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_HEADER,0L); 
	curl_easy_setopt(curl_handle, CURLOPT_HTTPGET,1); 
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT , 30L);
	curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT  , 30L);
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&proxy_ret);
	CURLcode  res = curl_easy_perform(curl_handle);
	if(res != CURLE_OK){
		fprintf(stderr, "[get_proxy_list::request] Failed[%s] errmsg[%s|%s]\n", 
				url.c_str(),errorBuffer,curl_easy_strerror(res));
		curl_easy_cleanup(curl_handle);
		return -4;
	}
	
	
	///判断返回的状态码
	int http_status = 0;
	res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE , &http_status);
	if(http_status != 200){
		fprintf(stderr, "[get_proxy_list::request] Failed[%s] errmsg[%s|%s] status[%d]\n", url.c_str(),errorBuffer,curl_easy_strerror(res),http_status);
		curl_easy_cleanup(curl_handle);
		return -4;
	}
    curl_easy_cleanup(curl_handle);
	
	
	//处理返回的结果
	Utils::trim(proxy_ret,"\n\r\t ");
	Utils::explode(proxy_ret,"\r\n",ret);
	fprintf(stderr,"[get_proxy_list::request] get %d proxy,ret[%s]\n", ret.size(),proxy_ret.c_str());
	
	return 0;
}
