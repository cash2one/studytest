#include "config_parser.h"

#define ARRAY_INIT_SZ 32

/**
 * 获取每目录配置的结构体指针
 */
void get_dir_conf(request_rec *r, passport_dir_conf** conf)
{
    *conf = (passport_dir_conf*) ap_get_module_config(r->per_dir_config, &newpassport_module);
}

/**
 * 获取本模块的每服务器配置
 */
void get_server_conf(request_rec *r, passport_server_conf** conf)
{
    *conf = (passport_server_conf*) ap_get_module_config(r->server->module_config, &newpassport_module);
}

/** 获取JPassportEnable*/
int is_passport_enabled(request_rec *r)
{
    passport_dir_conf* pdc = (passport_dir_conf*) ap_get_module_config(r->per_dir_config, &newpassport_module);
    return pdc->enabled;
}

/**
 * 是否启用了短用户名项
 */
int is_passport_short_username(request_rec *r)
{
    passport_dir_conf* pdc = (passport_dir_conf*) ap_get_module_config(r->per_dir_config, &newpassport_module);
    return pdc->enable_short_username;
}

/**
 * 获取IDP服务地址的端口
 */
short int get_idp_https_port(request_rec *r)
{
    short int port = 0;
    char strport[10];
    int i = 0;
    char* p1 = NULL;
    char* p2 = NULL;

    passport_server_conf* servconf = NULL;
    get_server_conf(r, &servconf);
    if(NULL == servconf){
        return 443;
    }

    char* url = servconf->authentication_url;
    if(url == NULL){
        url = "https://passport.sohu-rd.com/AuthorizationDecision";
    }

    p1 = strstr(url, ":");
    if(*(p1 + 1) == '/'){
        p1 += 3;
    }

    p1 = strstr(p1, ":");
    if(p1 == NULL){
        return 443;
    }

    p1++;
    p2 = strstr(p1, "/");
    if(p2 == NULL){
        p2 = url + strlen(url);
    }

    while(p1 != p2){
        strport[i] = *p1;
        i++;
        p1++;
    }
    strport[i] = '\0';

    port = atoi(strport);

    return port;
}

/**
 * 获取从passport.sogou-inc.com跳转过来的自动提交的地址，写死了，不让改
 */
int get_shire_addr(request_rec *r, char** shire_addr)
{
    unsigned int port = 0;
    char strport[10];
    const char* shire_uri = "/shire.jp";
    port = ap_get_server_port(r);
    sprintf(strport, ":%d", port);
    *shire_addr = apr_pstrcat(r->pool, "http://", r->hostname, strport, shire_uri, NULL);
    return 0;
}

/**
 * 获取IDP的授权主机名
 */
int get_idp_auth_hostname(request_rec *r, char** hostname)
{
    char* head = NULL;
    char* p1 = NULL;
    char* p2 = NULL;
    int i = 0;
    char* url = NULL;

    *hostname = (char*) apr_pcalloc(r->pool, 256);

    passport_server_conf* servconf = NULL;
    get_server_conf(r, &servconf);
    url = servconf->authentication_url;
    if(NULL == servconf || url == NULL){
        url = "https://passport.sogou-inc.com/AuthorizationDecision";
    }

    head = strstr(url, "//");
    head += 2;

    p1 = head;
    p2 = strstr(head, ":");
    if(p2 != NULL){
        while(p1 != p2){
            (*hostname)[i] = *p1;
            i++;
            p1++;
        }
    }
    else{
        p2 = strstr(head, "/");
        if(p2 == NULL){
            p2 = url + strlen(url);
        }

        while(p1 != p2){
            (*hostname)[i] = *p1;
            i++;
            p1++;
        }
    }

    return 0;
}

int get_idp_auth_path(request_rec* r, char** path)
{
    char* p1 = NULL;
    char* p2 = NULL;
    char* url = NULL;

    *path = (char*) apr_pcalloc(r->pool, 256);
    int i = 0;

    passport_server_conf* servconf = NULL;
    get_server_conf(r, &servconf);
    url = servconf->authentication_url;
    if(url == NULL){
        url = "https://passport.sogou-inc.com/AuthorizationDecision";
    }

    p1 = strstr(url, "//");
    p1++;
    p2 = strstr(p1 + 1, "/");
    if(p2 == NULL){
        return -1;
    }
    p2++;

    while(*p2 != '\0'){
        (*path)[i] = *p2;
        p2++;
        i++;
    }

    return 0;
}

/**是否允许启用本模块**/
const char* mod_newpassport_set_passport_enable(cmd_parms *cmd, void *mconfig, int arg)
{
    if(NULL == mconfig){
        fprintf(stderr, "[mod_newpassport]mod_newpassport_set_passport_enable error\n");
        return NULL;
    }
    passport_dir_conf *dcfg = (passport_dir_conf *) mconfig;
    dcfg->enabled = arg;
    return NULL;
}

const char* mod_newpassport_set_short_username(cmd_parms *cmd, void *mconfig, int arg)
{
    if(NULL == mconfig){
        fprintf(stderr, "[mod_newpassport]mod_newpassport_set_short_username error\n");
        return NULL;
    }
    passport_dir_conf *dcfg = (passport_dir_conf *) mconfig;
    dcfg->enable_short_username = arg;
    return NULL;
}

const char* mod_newpassport_set_filter(cmd_parms *cmd, void *mconfig, const char* arg)
{
    if(NULL == mconfig){
        fprintf(stderr, "[mod_newpassport]mod_newpassport_set_filter error\n");
        return NULL;
    }
    passport_dir_conf *dcfg = (passport_dir_conf *) mconfig;
    *(const char**) apr_array_push(dcfg->filter) = arg;
    return NULL;
}

const char* mod_newpassport_set_idp_url_address(cmd_parms *cmd, void *mconfig, const char *addr)
{
    passport_server_conf* scfg = (passport_server_conf *) ap_get_module_config(cmd->server->module_config, &newpassport_module);
    scfg->idp_addr = addr;
    return NULL;
}

const char* mod_newpassport_set_certificate_filename(cmd_parms *cmd, void *mconfig, const char *addr)
{
    passport_server_conf* scfg = (passport_server_conf *) ap_get_module_config(cmd->server->module_config, &newpassport_module);
    scfg->certificate_filename = addr;
    return NULL;
}

const char* mod_newpassport_set_authentication_url(cmd_parms *cmd, void *mconfig, const char *addr)
{
    passport_server_conf* scfg = (passport_server_conf *) ap_get_module_config(cmd->server->module_config, &newpassport_module);
    scfg->authentication_url = addr;
    return NULL;
}

const char* mod_newpassport_set_private_key_address(cmd_parms *cmd, void *mconfig, const char *addr)
{
    passport_server_conf* scfg = (passport_server_conf *) ap_get_module_config(cmd->server->module_config, &newpassport_module);
    scfg->private_key_addr = addr;
    return NULL;
}

const char* mod_newpassport_set_private_key_password(cmd_parms *cmd, void *mconfig, const char *addr)
{
    passport_server_conf* scfg = (passport_server_conf *) ap_get_module_config(cmd->server->module_config, &newpassport_module);
    scfg->private_key_password = addr;
    return NULL;
}

/** 创建每服务器配置*/
void *mod_newpassport_create_per_server(apr_pool_t *p, server_rec *d)
{
    passport_server_conf *scfg = NULL;
    if(p == NULL){
        fprintf(stderr, "[mod_newpassport]--------------Server config create error--------------------\n");
        return NULL;
    }
    else{
        scfg = (passport_server_conf*) apr_pcalloc(p, sizeof(passport_server_conf));
    }

    if(NULL == scfg){
        fprintf(stderr, "[mod_newpassport]=================Server config apr_pcalloc error==========================\n");
        return NULL;
    }

    scfg->idp_addr = NULL; // "http://passport.sohu-rd.com"
    scfg->certificate_filename = NULL; // "public.idp.sohu-rd.com.cer"
    scfg->authentication_url = NULL; // "https://passport.sohu-rd.com/AuthorizationDecision";
    scfg->private_key_addr = NULL;
    scfg->private_key_password = NULL;

    return scfg;
}

/** 创建每目录配置*/
void *mod_newpassport_create_per_dir(apr_pool_t *p, char *d)
{
    passport_dir_conf* dcfg = NULL;
    dcfg = (passport_dir_conf*) apr_pcalloc(p, sizeof(passport_dir_conf));
    if(NULL == dcfg){
        fprintf(stderr, "[mod_newpassport]mod_newpassport_create_per_dir error\n");
        return NULL;
    }
    dcfg->enabled = 0;
    dcfg->enable_short_username = 0;
    dcfg->filter = apr_array_make(p, ARRAY_INIT_SZ, sizeof(const char*));
    return dcfg;
}

/** 每目录配置合并函数*/
void *mod_newpassport_merge_per_dir(apr_pool_t *p, void *basev, void *addv)
{
    passport_dir_conf* base = (passport_dir_conf*) basev;
    passport_dir_conf* add = (passport_dir_conf*) addv;
    if(NULL == basev){
        fprintf(stderr, "[mod_newpassport]mod_newpassport_merge_per_dir basev error\n");
        return NULL;
    }
    if(NULL == addv){
        fprintf(stderr, "[mod_newpassport]mod_newpassport_merge_per_dir addv error\n");
        return NULL;
    }

    passport_dir_conf* new = NULL;
    new = apr_pcalloc(p, sizeof(passport_dir_conf));
    if(NULL == new){
        fprintf(stderr, "[mod_newpassport]mod_newpassport_merge_per_dir apr_palloc error\n");
        return NULL;
    }

    new->enabled = add->enabled;

    if(add->enable_short_username != 0)
        new->enable_short_username = add->enable_short_username;
    else
        new->enable_short_username = base->enable_short_username;

    if(add->filter->nelts != 0)
        new->filter = add->filter;
    else
        new->filter = base->filter;

    return new;
}

/** 每服务器合并函数*/
void *mod_newpassport_merge_per_server(apr_pool_t *p, void *basev, void *addv)
{
    if(NULL == basev){
        fprintf(stderr, "[mod_newpassport]mod_newpassport_merge_per_server basev error\n");
        return NULL;
    }
    if(NULL == addv){
        fprintf(stderr, "[mod_newpassport]mod_newpassport_merge_per_server addv error\n");
        return NULL;
    }
    passport_server_conf* base = (passport_server_conf*) basev;
    passport_server_conf* add = (passport_server_conf*) addv;
    passport_server_conf* new = NULL;

    new = apr_pcalloc(p, sizeof(passport_server_conf));
    if(NULL == new){
        fprintf(stderr, "[mod_newpassport]mod_newpassport_merge_per_server apr_palloc error\n");
        return NULL;
    }

    if(add->idp_addr != NULL){
        new->idp_addr = add->idp_addr;
    }
    else{
        new->idp_addr = base->idp_addr;
    }

    if(add->certificate_filename != NULL){
        new->certificate_filename = add->certificate_filename;
    }
    else{
        new->certificate_filename = base->certificate_filename;
    }

    if(add->private_key_addr != NULL){
        new->private_key_addr = add->private_key_addr;
    }
    else{
        new->private_key_addr = base->private_key_addr;
    }

    if(add->private_key_password != NULL){
        new->private_key_password = add->private_key_password;
    }
    else{
        new->private_key_password = base->private_key_password;
    }

    if(add->authentication_url != NULL){
        new->authentication_url = add->authentication_url;
    }
    else{
        new->authentication_url = base->authentication_url;
    }

    return new;
}
