#ifndef __MOD_PASSPORT_H__
#define __MOD_PASSPORT_H__

#include "httpd.h"

#include "http_config.h"
#include "http_request.h"

#define MAX_SIZE 2048
#define COOKIE_KEY_NAME "jpassport-sp"

#define VERIFY_ERROR          16000
#define SIGNATURE_ERROR       16001
#define COOKIE_ERROR          16002
#define AUTHORIZATION_ERROR   16003
#define CONFIG_ERROR          16004

#define VERIFY_ERROR_FILE          "jpassport_error_verify.html"
#define SIGNATURE_ERROR_FILE       "jpassport_error_sign.html"
#define COOKIE_ERROR_FILE          "jpassport_error_cookie.html"
#define AUTHORIZATION_ERROR_FILE   "jpassport_error_auth.html"
#define CONFIG_ERROR_FILE          "jpassport_error_config.html"

/**
 * 每目录配置
 */
typedef struct
{
    // PASSPORT ENABLED（ON/OFF）
    int enabled;

    int shortname;

    // PASSPORT ADDRESS
    const char* address;

    // shire的位置
    const char* shire_addr;
   
    // 私匙位置
    const char* private_key_addr;

    // 私匙密码
    const char* private_key_password;

    // 希望被过滤的文件扩展名
    apr_array_header_t* arrfilter;

} passport_dir_conf;

/**
 * 每服务器配置
 */
typedef struct
{
    // IDP 的URL
    const char* idp_addr;

    const char* client_path;

    const char* certificate_filename;

    const char* authentication_url;

} passport_server_conf;

/*********************************************************************/
/**          以下函数指针将引用libpassport.so中的相关函数            */

typedef int   (*verify_fn_t)(const char* samlresponsebuf, const char* certfiledir, const char* schemapath);

typedef int   (*get_saml_cookie_parameter_fn_t)(char*  response, 
                                                char** username, 
                                                char** notbefore, 
                                                char** notafter, 
                                                char** signvalue,
                                                const char* privkeyaddr, 
                                                const char* password);

typedef int   (*free_saml_cookie_paramter_fn_t)(const char* username,
                                                const char* notbefore,
                                                const char* notafter,
                                                const char* signvalue);

typedef char* (*sign_fn_t)(const char* msg, const char* private_key_addr, const char* password);

typedef int   (*get_decision_fn_t)(char* username, char* resource, char* action, char* idp_addr, char* idp_path, unsigned int port);

typedef char* (*compress_fn_t)(char* signature);

typedef char* (*uncompress_fn_t)(const char* base64signcompressed);


extern module MODULE_VAR_EXPORT      passport_module;

/*********************************************************************/

int   is_passport_enabled (request_rec *r);

const char* get_passport_address(request_rec *r);

int is_shortusername(request_rec* r);

const char* get_private_key_addr(request_rec *r);

const char* get_certificate_addr(request_rec *r);

const char* get_libpassport_addr(server_rec *r);

const char* get_saml_schema_addr(request_rec *r);

const char* get_shire_addr(request_rec *r);

const char* get_idp_addr(request_rec *r);

short int   get_idp_https_port(request_rec *r);

const char* get_idp_auth_hostname(request_rec* r);

const char* get_idp_auth_path(request_rec* r);

#define ARRAY_INIT_SZ  32
apr_array_header_t* get_filter_array(request_rec *r);

int   filter(request_rec *r);

/*********************************************************************/
/**                       与配置相关的函数                           */
void* mod_passport_create_per_dir(apr_pool_t *p, char *d);

void* mod_passport_merge_per_dir(apr_pool_t *p, void *basev, void *addv);

void* mod_passport_create_per_server(apr_pool_t *p, server_rec *d);

void* mod_passport_merge_per_server(apr_pool_t *p, void *basev, void *addv);
/*********************************************************************/

const char* mod_passport_set_passport_enable(cmd_parms *cmd, void *mconfig, int arg);

const char* mod_passport_short_username(cmd_parms *cmd, void *mconfig, int arg);

const char* mod_passport_set_filter_string(cmd_parms *cmd, void *mconfig, const char *addr);

const char* mod_passport_set_passport_address(cmd_parms *cmd, void *mconfig, const char *addr);

const char* mod_passport_set_shire_address(cmd_parms *cmd, void *mconfig, const char *addr);

const char* mod_passport_set_private_key_address(cmd_parms *cmd, void *mconfig, const char *addr);

const char* mod_passport_set_idp_url_address(cmd_parms *cmd, void *mconfig, const char *addr);

const char* mod_passport_set_client_path(cmd_parms *cmd, void *mconfig, const char *addr);

const char* mod_passport_set_private_key_password(cmd_parms *cmd, void *mconfig, const char *addr);

const char* mod_passport_set_certificate_filename(cmd_parms *cmd, void *mconfig, const char *addr);

const char* mod_passport_set_authentication_url(cmd_parms *cmd, void *mconfig, const char *addr);
/*********************************************************************/