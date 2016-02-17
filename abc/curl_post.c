#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<curl/curl.h>


#define URL "http://test.wendao.com/"

int main(int argc, char* argv[]){
	CURL *curl;
	CURLcode res;
	struct curl_slist *headers=NULL;
	curl = curl_easy_init();
	if(!curl){
		return -1;
	}
	

	headers = curl_slist_append(headers, "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3");
	headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:30.0) Gecko/20100101 Firefox/30.0");
	headers = curl_slist_append(headers, "Connection: keep-alive");
	headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	headers = curl_slist_append(headers, "Accept-Charset: utf-8,gb2312;q=0.7,*;q=0.7");
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip,deflate");
	curl_easy_setopt(curl, CURLOPT_URL, URL);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 3L);
	curl_easy_setopt(curl, CURLOPT_HEADER,0L); 
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");
	
	//const char *postthis="moo mooo moo moo";
	//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);
	//curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postthis));
	
	res = curl_easy_perform(curl);
    if(res != CURLE_OK){
		fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
	}
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);

    return 0;
}
