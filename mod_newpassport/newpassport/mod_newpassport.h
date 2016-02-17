#ifndef _MOD_NEWPASSPORT_H_
#define _MOD_NEWPASSPORT_H_
#include "httpdh.h"


extern module AP_MODULE_DECLARE_DATA newpassport_module;

typedef struct {
    char* idp_addr;
    char* certificate_filename;
    char* authentication_url;
    char* private_key_addr;
    char* private_key_password;
} passport_server_conf;

typedef struct {
    int enabled;
    int enable_short_username;
    apr_array_header_t* filter;
} passport_dir_conf;

typedef struct {
    char* username; //从SAML里获取的用户名
    char* notbefore; //从SAML里获取的时间1
    char* notafter; //从SAML里获取的时间2
    char* sign; //根据上面三个字段计算出来的签名值，要放到cookie里的
    char* post_raw_data; //POST过来的原始数据
    char* saml_str; //解码后的SAML文档字符串
    char* target; //要跳转的地址
    char* json_cookie; //拼接好的json形式的cookie内容
    char* SignatureValue; //从SAML里获取的SignatureValue的值,是对signedInfo节点签名后的值
    char* DigestValue; //从SAML里获取的DigestValue的值，是对包含断言信息的SAML文档的摘要信息
} saml_elem;

typedef struct {
    char* username; //从cookie中提到的用户名
    char* notbefore; //从cookie中提取的时间1
    char* notafter; //从cookie中提取的时间2
    char* cookie_sign; //从cookie提取的签名值
    char* regenerate_sign; //根据username、notbefore、notafter重新计算的签名值，要跟cookie_sign对比的
    char* uncompressed_cookie_signvalue; //对cookie_sign解压后得到的值，其实真正对比的是这样值
} cookie_elem;

#define ID_NAME "ResponseID"
#define C14N_ALGORITHM "http://www.w3.org/2001/10/xml-exc-c14n#"
#define SIGNATURE_ALGORITHM "http://www.w3.org/2000/09/xmldsig#rsa-sha1"
#define DIGEST_ALGORITHM "http://www.w3.org/2000/09/xmldsig#sha1"
#define TRANSFORM_ENVELOP "http://www.w3.org/2000/09/xmldsig#enveloped-signature"


void set_redirect(request_rec *r, const char *new_url);
void log_error(request_rec *r, const char* msg);
int get_cookie(request_rec *r, const char* cookie_name, char** cookie_value);
int get_post_content(request_rec* r, saml_elem* se);
int get_saml_from_post(apr_pool_t* pool, saml_elem* se);
int set_cookie(request_rec* r, saml_elem* se);
int is_post_shire(request_rec* r);
int get_target_from_post(apr_pool_t* pool, saml_elem* se);
int get_target_from_get(request_rec* r, char** target);
int get_redirect_url(request_rec* r, const char* target, const char* shire, char** redirect_url);
int is_filter(request_rec* r);

int get_saml_elem_value(request_rec *r, saml_elem* se);
int get_json_cookie_str(request_rec *r, saml_elem* se);
int verify_saml_response(request_rec* r, const char* responsexml);

#endif