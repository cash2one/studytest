#include "utils.h"
#include "config_parser.h"
#include <time.h>


#define COOKIE_KEY_NAME "jpassport-sp"
#define MAX_SIZE 4096

/**
 *  设置request表头的Http和Location项，从而重定向至指定的页面
 */
void set_redirect(request_rec *r, const char *new_url)
{
    apr_table_setn(r->err_headers_out, "Http", "302");
    apr_table_setn(r->err_headers_out, "Location", new_url);
}

/**
 * 记录错误日志信息
 */
void log_error(request_rec *r, const char* msg)
{
    fprintf(stderr, msg);
    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, msg);
}

/**
 *  得到HTTP请求头中的Cookie信息
 */
int get_cookie(request_rec *r, const char* cookie_name, char** cookie_buf)
{
    const char* cookie_str = NULL;
    char *value = NULL;

    // 从请求报头中得到
    // ap_table_get将调用strdup返回给cookie_str
    cookie_str = (char*) apr_table_get(r->headers_in, "Cookie");
    if(NULL == cookie_str){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_newpassport] get cookie from headers failed\n");
        return -1;
    }

    //为了避免其他cookie的干扰
    value = strstr(cookie_str, COOKIE_KEY_NAME);
    if(NULL == value){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_newpassport] get position of %s from %s failed\n", COOKIE_KEY_NAME, cookie_str);
        return -1;
    }

    //cookie字段的位置
    value = strstr(value, cookie_name);
    if(NULL == value){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_newpassport] get position of %s from %s failed\n", cookie_name, value);
        return -1;
    }

    char* cookie_end;

    value += strlen(cookie_name) + 1; // 还有一个冒号
    *cookie_buf = apr_pstrdup(r->pool, value);
    if(strcmp(cookie_name, "sign") != 0){
        cookie_end = strchr(*cookie_buf, ',');
    }
    else{
        cookie_end = strchr(*cookie_buf, '}');
    }

    if(cookie_end){
        *cookie_end = '\0';
    }

    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "[mod_newpassport] get cookie success:%s=%s\n",cookie_name,*cookie_buf);
    return 0;
}

/**
 * 拼凑设置Cookie的内容
 */
int get_json_cookie_str(request_rec *r, saml_elem* se)
{
    if(!se->username || !se->notbefore || !se->notafter || !se->sign){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_newpassport] canot get json cookie str,some field is empty\n");
        return -1;
    }

    ////压缩
    char* compressed_signature = NULL;
    int comRet = compress_cookie_signature(r, se->sign, &compressed_signature);
    if(0 != comRet){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, comRet, r, "[mod_newpassport] compress_cookie_signature failed\n");
        return -1;
    }

    se->json_cookie = apr_psprintf(r->pool, "{username:%s,timebefore:%s,notonorafter:%s,sign:%s}",
                                   se->username, se->notbefore, se->notafter, compressed_signature);
    return 0;
}

/**
 * 获取请求过来的POST的内容：samlResponse=XXX&target=XXXX
 */
int get_post_content(request_rec* r, saml_elem* se)
{
    int bytes = 0;
    int eos = 0;
    apr_size_t count = 0;
    apr_status_t rv;
    apr_bucket_brigade* bb;
    apr_bucket_brigade* bbin;

    apr_bucket* b;

    const char* clen = apr_table_get(r->headers_in, "Content-Length");
    if(clen != NULL){
        bytes = strtol(clen, NULL, 0);
    }
    else{
        bytes = MAX_SIZE;
    }

    bb = apr_brigade_create(r->pool, r->connection->bucket_alloc);
    bbin = apr_brigade_create(r->pool, r->connection->bucket_alloc);

    do{
        rv = ap_get_brigade(r->input_filters, bbin, AP_MODE_READBYTES, APR_BLOCK_READ, bytes);
        if(rv != APR_SUCCESS){
            ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "[mod_newpassport] Failed to read from input!\n");
            return -1;
        }

        for(b = APR_BRIGADE_FIRST(bbin); b != APR_BRIGADE_SENTINEL(bbin);){
            if(APR_BUCKET_IS_EOS(b)){
                eos = 1;
            }

            if(!APR_BUCKET_IS_METADATA(b)){
                apr_bucket* temp = b;
                count += b->length;
                b = APR_BUCKET_NEXT(b);
                APR_BUCKET_REMOVE(temp);
                APR_BRIGADE_INSERT_TAIL(bb, temp);
            }
            else{
                b = APR_BUCKET_NEXT(b);
            }
        }
    }while(!eos);

    se->post_raw_data = apr_pcalloc(r->pool, count + 1);
    apr_brigade_flatten(bb, se->post_raw_data, &count);

    return 0;
}

/**
 * 获取POST过来的内容里的samlResponse的值，经过base64_decode返回saml格式的内容
 */
int get_saml_from_post(apr_pool_t* pool, saml_elem* se)
{
    const char* samlpre = "samlResponse=";
    char* p = NULL;

    ap_unescape_url(se->post_raw_data);
    p = strstr(se->post_raw_data, samlpre);
    if(p == NULL){
        return -1;
    }
    p += strlen(samlpre);
    remove_lineends(p);
    se->saml_str = ap_pbase64decode(pool, p);

    return 0;
}

/**
 * 提取POST过来的信息里提取target字段的值
 */
int get_target_from_post(apr_pool_t* pool, saml_elem* se)
{
    char* needle = "target=";
    char* p = NULL;
    char* q = NULL;
    int i = 0;

    p = strstr(se->post_raw_data, needle);
    if(p == NULL){
        return -2;
    }
    p += strlen(needle);

    q = strstr(se->post_raw_data, "&");
    if(q == NULL){
        return -1;
    }

    se->target = (char*) apr_pcalloc(pool, (q - p + 1) * sizeof(char));

    while(*p != *q){
        (se->target)[i] = *p;
        i++;
        p++;
    }

    ap_unescape_url(se->target);

    return 0;
}

/**
 * 设置Cookie
 */
int set_cookie(request_rec* r, saml_elem* se)
{
	//将形如2014-06-18T13:47:07.710Z这样的日期转化为GMT形式Wed, 09-Jul-2014 09:06:41 GMT
	struct tm gmt;
	gmt.tm_year = atoi(se->notafter) - 1900;
	gmt.tm_mon = atoi(se->notafter + 5) -1;
	gmt.tm_mday = atoi(se->notafter + 8);
	gmt.tm_hour = atoi(se->notafter + 11);
	gmt.tm_min = atoi(se->notafter + 14);
	gmt.tm_sec = atoi(se->notafter + 17);
	time_t a = mktime(&gmt);
	memcpy(&gmt,localtime(&a),sizeof(struct tm));
	char* expire = (char*)apr_pcalloc(r->pool,sizeof(char)*50);
	strftime(expire,50,"%a, %d-%b-%Y %H:%M:%S GMT",&gmt);
	
    char* new_cookie = NULL;
    new_cookie = apr_pstrcat(r->pool, "jpassport-sp=", se->json_cookie, ";expires=",expire,";path=/;", NULL);
    apr_table_setn(r->err_headers_out, "Set-Cookie", new_cookie);

    return 0;
}

/**
 * 判断是否为passport.sogou-inc.com重定向后POST过来的请求
 */
int is_post_shire(request_rec* r)
{
    char* p = NULL;
    p = strstr(r->uri, "shire");
    if(!p){
        return 0;
    }

    return 1;
}

/**
 * 得到用户请求的URL，即target
 */
int get_target_from_get(request_rec* r, char** target)
{
    char strport[10];
    unsigned int port;
    port = ap_get_server_port(r);
    sprintf(strport, ":%d", port);

    *target = apr_pstrcat(r->pool, "http://", r->hostname, strport, r->uri, NULL);

    if(r->args != NULL){
        *target = apr_pstrcat(r->pool, *target, "?", r->args, NULL);
    }
    return 0;
}

/**
 * 得到重定向到IDP时的URL
 * GET时专用
 */
int get_redirect_url(request_rec* r, const char* target, const char* shire, char** redirect_url)
{
    passport_server_conf* servconf = NULL;
    get_server_conf(r, &servconf);
    const char* idp_addr = servconf->idp_addr;
    *redirect_url = apr_pstrcat(r->pool, idp_addr, "?shire=", shire, "&target=", target, NULL);
    return 0;
}

/**
 * 按uri进行过滤
 */
int is_filter(request_rec* r)
{
    passport_dir_conf* conf = NULL;
    get_dir_conf(r, &conf);
    if(NULL == conf){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_newpassport] get passport dir conf structure failed\n");
        return 0;
    }

    //获取这些过滤规则数组
    apr_array_header_t* filter_array = conf->filter;

    //没有设置过滤规则，不过滤
    if(filter_array == NULL || filter_array->nelts == 0){
        ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "[mod_newpassport] the filter rule is empty\n");
        return 0;
    }

    //按正则表达式，逐个过滤
    int i;
    ap_regex_t *regexp;
    const char* uri_reg = NULL;
    for(i = 0; i < filter_array->nelts; i++){
        uri_reg = ((const char**) filter_array->elts)[i];
        regexp = ap_pregcomp(r->pool, uri_reg, AP_REG_EXTENDED);
        if(ap_regexec(regexp, r->uri, 0, NULL, 0) == 0){
            ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "[mod_newpassport] filtered uri: %s\t%s\n", r->uri, uri_reg);
            return 1;
        }
    }

    return 0;
}