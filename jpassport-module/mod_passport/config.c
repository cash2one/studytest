/*
 **
 ** �ļ�����config.c
 ** ʵ��JPassport-Client-Apache-Module��httpd.conf������
 ** ���ߣ���գ�Unnie Feng��
 **
 */

#include "httpd.h"
#include "http_config.h"
#include "http_log.h"
#include "ap_config.h"
#include "mod_passport.h"

#include "apr_strings.h"
#include "apr_strmatch.h"


#include "http_core.h"

/**
 * ��ȡ��ģ���ÿĿ¼����
 */
passport_dir_conf* get_dir_conf(request_rec *r)
{
    return (passport_dir_conf*)ap_get_module_config(r->per_dir_config,&passport_module);
}

/**
 * ��ȡ��ģ���ÿ����������
 */
passport_server_conf* get_server_conf(request_rec *r)
{
    return (passport_server_conf*)ap_get_module_config(r->server->module_config,&passport_module);
}

/*
 **
 ** ����ͨ�����º�����ȡ�����������ֵ
 **
 */

/** ��ȡJPassportRootPathURI*/
const char* get_passport_addr(request_rec *r)
{
    char* rootpath = get_dir_conf(r)->address;
    if(rootpath == NULL) {
		return "/";
	}
    return rootpath;
}

/** ��ȡJPassportEnable*/
int is_passport_enabled(request_rec *r)
{
    return get_dir_conf(r)->enabled;
}

/** ��ȡJPassportPrivateKeyFileName*/
const char* get_private_key_addr(request_rec *r)
{
    passport_server_conf* scfg = get_server_conf(r);
    if(get_dir_conf(r)->private_key_addr == NULL){
       get_dir_conf(r)->private_key_addr = "qadev.sohu-rd.com.key";
	}
	
    return apr_pstrcat(r->pool, scfg->client_path, "/", get_dir_conf(r)->private_key_addr, NULL);
}

/** ��ȡJPassportCertificateFileName*/
const char* get_certificate_addr(request_rec *r)
{
    char* ceraddr = NULL;
    char* cername = NULL;
    passport_server_conf* scfg = get_server_conf(r);

    cername = scfg->certificate_filename;
    if(cername == NULL) {
		cername = "public.idp.sohu-rd.com.cer";
	}

    ceraddr = apr_pstrcat(r->pool, scfg->client_path, "/", cername, NULL);
    return ceraddr;
}

short int get_idp_https_port(request_rec *r)
{
    short int port = 0;
    char  strport[10];
    int i = 0;
    char* p1 = NULL;
    char* p2 = NULL;

    char* url = get_server_conf(r)->authentication_url;
    if(url == NULL) {
		url = "https://passport.sohu-rd.com/AuthorizationDecision";
	}

    p1 = strstr(url, ":");
    if(*(p1+1) == '/') {
		p1 += 3;
	}

    p1 = strstr(p1, ":");
    if(p1 == NULL) {
		return 443;
	}

    p1++;
    p2 = strstr(p1, "/");
    if(p2 == NULL) {
		p2 = url + strlen(url);
	}

    while(p1 != p2)
    {
        strport[i] = *p1;
        i++;
        p1++;
    }
    strport[i] = '\0';

    port = atoi(strport);

    return port;
}

const char* get_idp_auth_hostname(request_rec *r)
{
    char* head  = NULL;
    char* p1    = NULL;
    char* p2    = NULL;
    int   i     = 0;
    char* url   = NULL;
    static char hostname[150];
    hostname[0] = '\0';

    url = get_server_conf(r)->authentication_url;
    if(url == NULL) {
		url = "https://passport.sohu-rd.com/AuthorizationDecision";
	}

    head = strstr(url, "//");
    head += 2;

    p1 = head;
    p2 = strstr(head, ":");
    if(p2 != NULL)
    {
       while(p1 != p2)
       {
           hostname[i] = *p1;
           i++;
           p1++;
       }
       hostname[i] = '\0';
    }
    else
    {
       p2 = strstr(head, "/");
       if(p2 == NULL) p2 = url + strlen(url);

       while(p1 != p2)
       {
           hostname[i] = *p1;
           i++;
           p1++;
       }

       hostname[i] = '\0';
    }

    return hostname;
}

const char* get_idp_auth_path(request_rec* r)
{
    char* p1    = NULL;
    char* p2    = NULL;
    char* url   = NULL;
    static char path[150];
    int i = 0;

    url = get_server_conf(r)->authentication_url;
    if(url == NULL) {
		url = "https://passport.sohu-rd.com/AuthorizationDecision";
	}

    p1 = strstr(url, "//");
	p1++;
    p2 = strstr(p1 + 1, "/");
	if(p2 == NULL) {
		return "";
	}
    p2++;

    while(*p2 != '\0')
    {
        path[i] = *p2;
        p2++;
        i++;
    }

    return path;
}

/** ��ȡJPassportLibFileName*/
const char* get_libpassport_addr(server_rec *s)
{
    static char libpassportaddr[150];
    libpassportaddr[0] = '\0';
    passport_server_conf* scfg = ap_get_module_config(s->module_config,&passport_module);
    strcpy(libpassportaddr, scfg->client_path);
    strcat(libpassportaddr, "/modules/libpassport.so");

    return libpassportaddr;
}

/** ��ȡJPassportSAMLSchemaAddress*/
const char* get_saml_schema_addr(request_rec *r)
{
    passport_server_conf* scfg = ap_get_module_config(r->server->module_config,&passport_module);
    return apr_pstrcat(r->pool, scfg->client_path, "/schemas", NULL);
}

/** ��ȡJPassportIDPUrl*/
const char* get_idp_addr(request_rec *r)
{
    char* idp =  get_server_conf(r)->idp_addr;
    if(idp == NULL) {
		return "http://passport.sohu-rd.com";
	}

    return idp;
}

/**
 * �õ����������ʾ��Ϣ���ļ���
 * �õ����ļ���ָ�벻��Ҫ���ͷ�
 */
char* get_error_doc_filename(int err_case)
{
   char* filename = NULL;

   switch (err_case)
   {
      case VERIFY_ERROR:filename = VERIFY_ERROR_FILE;break;
      case SIGNATURE_ERROR:filename = SIGNATURE_ERROR_FILE;break;
      case COOKIE_ERROR:filename = COOKIE_ERROR_FILE;break;
      case AUTHORIZATION_ERROR:filename = AUTHORIZATION_ERROR_FILE;break;
      case CONFIG_ERROR:filename = CONFIG_ERROR_FILE;break;
      default:
           filename = NULL;
      break;
   }

   return filename;
}

/**
 * �õ������ĵ�������URL
 * ����ָ��JPassportErrorDocumentURI���ô����ĵ�
 * ����ڷ�������Ŀ¼��·��
 */
char* get_error_doc_addr(request_rec *r, int err_case)
{
   return NULL;
}

/** ��ȡJPassportPrivateKeyFileName*/
const char* get_private_key_password(request_rec *r)
{
    char* password = get_dir_conf(r)->private_key_password;
    if(password == NULL) {
		return "11111111";
	}

    return password;
}

int is_shortusername(request_rec* r)
{
    int isshort =  get_dir_conf(r)->shortname;
    return isshort;
}

/**
 * �õ�shire��ַ
 * ����shire������URL
 * ����ָ�JPassportShireURI
 */
const char* get_shire_addr(request_rec *r)
{
    unsigned int port = 0;
    char  strport[10];
    char* complete_shire_url = NULL;
    const char* shire_uri    = NULL;
    char* root_path_uri = get_dir_conf(r)->address;
    if(root_path_uri == NULL) {
		root_path_uri = "/";
	}
    char* shire_name    = get_dir_conf(r)->shire_addr;
    if(shire_name == NULL) {
		shire_name = "shire.jp"; // Ĭ��ֵ
	}

    // �ж�JPassportRootPathURI�Ƿ�����"/"��β��
    if(root_path_uri[strlen(root_path_uri) - 1] != '/')
    {
       shire_uri = apr_pstrcat(r->pool,root_path_uri, "/",shire_name,NULL);
    }
    else
    {
       shire_uri = apr_pstrcat(r->pool,root_path_uri,shire_name,NULL);
    }

    if(shire_uri == NULL) {
		return NULL;
	}

    port = ap_get_server_port(r);
    sprintf(strport, ":%d", port);

    if(shire_uri[0] == '/'){
       complete_shire_url = apr_pstrcat(r->pool,"http://", r->hostname, strport, shire_uri, NULL);
	}
    else{
       complete_shire_url = apr_pstrcat(r->pool,"http://", r->hostname, strport, "/", shire_uri, NULL);
	}

    return complete_shire_url;
}

static
const char* get_shire_filename(request_rec* r) 
{
    int shire_str_len = 0;
    const char* shire_addr  = NULL;

    shire_addr = get_shire_addr(r);
    if(shire_addr != NULL)
    {
        int   i = 0;;
        const char* p = NULL;
        shire_str_len = strlen(shire_addr);
        for(i = shire_str_len - 1; i >= 0; i--)
        {
            if(shire_addr[i] == '/' || i == 0)
            {
                break;
            }
        }

        if(i != 0){
            p = &shire_addr[i] + 1;
		}
        else{
            p = &shire_addr[i];
		}

        return p;
    }

    return NULL;
}

apr_array_header_t* get_filter_array(request_rec *r)
{
    return get_dir_conf(r)->arrfilter;
}

/**
 * ����ļ����ַ����Ƿ���ָ����չ����β
 * filename���ļ����ַ���
 * ext����չ���ַ���
 * ���filename����extΪ��β�ģ��򷵻�1,
 * ���򷵻�0
 */
static
int check_extension(const char* filename,const char* ext)
{
    int extlen       = 0;
    int filename_len = 0;
    const char* p    = NULL;

    extlen = strlen(ext);
    filename_len = strlen(filename);

    if(extlen == 0 || filename == 0){
        return 0;
	}

    p = filename + filename_len - extlen;
    if(strcmp(p, ext) == 0){
        return 1;
	}

    return 0;
}

/**
 * �ж��û�����URI�Ƿ���JPassportFilterָ���Ĺ�������
 * ����������һ��������ʽ
 * ���� ��.*\.jsp login://56 access��
 * ������ֻ���ǵ�һ���ո�֮ǰ��������ʽ
 */
int filter(request_rec* r)
{
    int i;
    apr_array_header_t* filter_array = NULL;

    //// ����shire����ʲô��ʽ�����Ķ����뱻����
    if(check_extension(r->uri, get_shire_filename(r))){
        return 1;
	}

    // ����JPassportFilter��ÿĿ¼����
    filter_array = get_filter_array(r);

    // û�����þ���ȫ������
    if(filter_array == NULL || filter_array->nelts == 0){
        return 1;
	}

    for (i = 0; i < filter_array->nelts; i++)
    {
        const char* s = NULL;
        char* p1 = NULL;
        char* p2 = NULL;
        char  str[100];
        int   j = 0;

        ap_regex_t *regexp;

        str[0] = '\0';
        s = ((const char**)filter_array->elts)[i];
        p1 = s;
        p2 = strstr(s, " ");
        if(p2 != NULL)
        {
            while(p1 != p2)
            {
                str[j] = *p1;
                j++;
                p1++;
            }
            str[j] = '\0';
        }
        else{
            strcpy(str, s);
		}

        regexp = ap_pregcomp(r->pool, str, AP_REG_EXTENDED);
        if(ap_regexec(regexp, r->uri, 0, NULL, 0) == 0)
        {
            return 1;
        }
   }

   return 0;
}

/*
 **
 ** ���º�������command_rec��ע�������ָ����غ���
 **
 */

/** ����ÿĿ¼����*/
void *mod_passport_create_per_dir(apr_pool_t *p, char *d)
{
    passport_dir_conf* dcfg;

    dcfg = (passport_dir_conf*) apr_pcalloc(p, sizeof(passport_dir_conf));

    dcfg->enabled   = 0;
    dcfg->shortname = 0;
    dcfg->address   = NULL; // "/"
    dcfg->private_key_addr     = NULL;   // "qadev.sohu-rd.com.key"
    dcfg->shire_addr           = NULL;   // "shire.jp"
    dcfg->private_key_password = NULL;   // "11111111"
    dcfg->arrfilter = apr_array_make(p, ARRAY_INIT_SZ, sizeof(const char*));

    return dcfg;
}

/** ÿĿ¼���úϲ�����*/
void *mod_passport_merge_per_dir(apr_pool_t *p, void *basev, void *addv)
{
    passport_dir_conf* base = (passport_dir_conf*)basev;
    passport_dir_conf* add  = (passport_dir_conf*)addv;

    passport_dir_conf* new = NULL;

    new = apr_palloc(p, sizeof(passport_dir_conf));
    new->enabled = add->enabled;

    if(add->private_key_addr != NULL){
       new->private_key_addr = add->private_key_addr;
	}
    else{
       new->private_key_addr = base->private_key_addr;
	}

    if(add->shortname != 0){
       new->shortname = add->shortname;
	}
    else{
       new->shortname = base->shortname;
	}

    if(add->address != NULL){
       new->address = add->address;
	}
    else{
       new->address = base->address;
	}

    if(add->shire_addr != NULL){
       new->shire_addr = add->shire_addr;
	}
    else{
       new->shire_addr = base->shire_addr;
	
     // �ж��Ƿ�һ��Ԫ��Ҳû��
    if(add->arrfilter->nelts != 0){
       new->arrfilter = add->arrfilter;
	}
    else{
       new->arrfilter = base->arrfilter;
	}

    return new;
}

/** ����ÿ����������*/
void *mod_passport_create_per_server(apr_pool_t *p, server_rec *d)
{
    passport_server_conf *scfg;

    if(p == NULL){
       fprintf(stderr, "[mod_passport]Server config create error\n");
       return NULL;
    }
    else{ 
		scfg = (passport_server_conf*)apr_pcalloc(p, sizeof(passport_server_conf));
	}

    scfg->idp_addr             = NULL; // "http://passport.sohu-rd.com"
    scfg->client_path          = NULL; // "/usr/local/passport_apache_2x/"
    scfg->certificate_filename = NULL; // "public.idp.sohu-rd.com.cer"
    scfg->authentication_url   = NULL; // "https://passport.sohu-rd.com/AuthorizationDecision";

    return scfg;
}

/** ÿ�������ϲ�����*/
void *mod_passport_merge_per_server(apr_pool_t *p, void *basev, void *addv)
{
    passport_server_conf* base = (passport_dir_conf*)basev;
    passport_server_conf* add  = (passport_dir_conf*)addv;
    passport_server_conf* new  = NULL;

    new = apr_palloc(p, sizeof(passport_server_conf));

    if(add->idp_addr != NULL){
        new->idp_addr = add->idp_addr;
	}
    else{
        new->idp_addr = base->idp_addr;
	}

    if(add->client_path != NULL){
        new->client_path = add->client_path;
	}
    else{
        new->client_path = base->client_path;
	}

    if(add->certificate_filename != NULL){
        new->certificate_filename = add->certificate_filename;
	}
    else{
        new->certificate_filename = base->certificate_filename;
	}

    if(add->authentication_url != NULL){
        new->authentication_url = add->authentication_url;
	}
    else{
        new->authentication_url = base->authentication_url;
	}

    return new;
}

const char *mod_passport_set_passport_enable(cmd_parms *cmd, void *mconfig, int arg)
{
    passport_dir_conf *dcfg = (passport_dir_conf *)mconfig;
    dcfg->enabled = arg;
  
    return NULL;
}

const char* mod_passport_short_username(cmd_parms *cmd, void *mconfig, int arg)
{
    passport_dir_conf *dcfg = (passport_dir_conf *)mconfig;
    dcfg->shortname = arg;

    return NULL;
}

const char *mod_passport_set_passport_address(cmd_parms *cmd, void *mconfig, const char *addr)
{
    passport_dir_conf *dcfg = (passport_dir_conf *)mconfig;
    dcfg->address = addr;

    return NULL;
}

const char* mod_passport_set_shire_address(cmd_parms *cmd, void *mconfig, const char *addr)
{
   passport_dir_conf *dcfg = (passport_dir_conf *)mconfig; 
   dcfg->shire_addr = addr;

   return NULL;
}

const char* mod_passport_set_idp_url_address(cmd_parms *cmd, void *mconfig, const char *addr)
{
    passport_server_conf* scfg = (passport_server_conf *)ap_get_module_config(cmd->server->module_config,&passport_module);
    scfg->idp_addr = addr;
    
    return NULL;
}

const char* mod_passport_set_client_path(cmd_parms *cmd, void *mconfig, const char *addr)
{
    passport_server_conf* scfg = (passport_server_conf *)ap_get_module_config(cmd->server->module_config,&passport_module);
    scfg->client_path = addr;

    return NULL;
}

const char* mod_passport_set_certificate_filename(cmd_parms *cmd, void *mconfig, const char *addr)
{
    passport_server_conf* scfg = (passport_server_conf *)ap_get_module_config(cmd->server->module_config,&passport_module);
    scfg->certificate_filename = addr;

    return NULL;
}

const char* mod_passport_set_authentication_url(cmd_parms *cmd, void *mconfig, const char *addr)
{
    passport_server_conf* scfg = (passport_server_conf *)ap_get_module_config(cmd->server->module_config,&passport_module);
    scfg->authentication_url = addr;
    
    return NULL;
}

const char* mod_passport_set_private_key_address(cmd_parms *cmd, void *mconfig, const char *addr)
{
    passport_dir_conf *dcfg = (passport_dir_conf *)mconfig; 
    dcfg->private_key_addr = addr;

    return NULL;
}

const char* mod_passport_set_private_key_password(cmd_parms *cmd, void *mconfig, const char *addr)
{
    passport_dir_conf *dcfg = (passport_dir_conf *)mconfig;
    dcfg->private_key_password = addr;

    return NULL;
}

const char* mod_passport_set_filter_string(cmd_parms *cmd, void *mconfig, const char *addr)
{
    passport_dir_conf *dcfg = (passport_dir_conf *)mconfig;
    *(const char**)apr_array_push(dcfg->arrfilter) = addr;

    return NULL;
}