/*
 **
 ** �ļ�����mod_passport.c
 ** ʵ��JPassport-Client-Apache-Module
 ** ���ߣ���գ�Unnie Feng��
 ** ��ϵ��ʽ��unniefeng@sohu-rd.com
 **
 */

#include "httpd.h"

#include "http_config.h"
#include "http_request.h"
#include "http_protocol.h"
#include "http_log.h"

#include "http_core.h"
#include "http_main.h"
#include "util_script.h"


#include "apr_general.h"
#include "apr_strings.h"

#include "ap_config.h"

#include "mod_passport.h"
#include <stdio.h>

// libpassport.so��APR DSO���
static apr_dso_handle_t*   dso_h = NULL;

// ������֤�����Ƿ����������IDP
static verify_fn_t                     verify_fn       = NULL;
// ����õ��û���(username)��Cookie����
static get_saml_cookie_parameter_fn_t  cookie_param_fn = NULL;
// �����Cookie�е����ݽ���ǩ��
static sign_fn_t                       sign_fn         = NULL;
// ����õ��û�Ȩ��
static get_decision_fn_t               get_decision_fn = NULL;
// ����ѹ��Cookie����
static compress_fn_t                   compress_fn     = NULL;
// �����ѹ������
static uncompress_fn_t                 uncompress_fn   = NULL;
// �����ͷ�Cookie����
static free_saml_cookie_paramter_fn_t  free_saml_cookie_paramter_fn = NULL;

/**
 *  ����request��ͷ��Http��Location��Ӷ��ض�����ָ����ҳ��
 *  r ��Ӧ����
 *  new_url �ض���ҳ���URL
 */
static void set_redirect(request_rec *r,const char *new_url)
{
    /* ����Ӧ���MIME��ͷ����*/
    apr_table_setn(r->err_headers_out, "Http", "302");
    apr_table_setn(r->err_headers_out, "Location", new_url);
}

static void log_error(request_rec *r, const char* msg)
{
    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, msg);
}

/**
 *  �õ�HTTP����ͷ�е�Cookie��Ϣ
 *  ����ָ��Cookie��ָ��
 *  ע��!! �����ַ�����Ҫ��free�ͷ�
 *  �μ������free_cookie_param����,
 *  �õ����ַ��������ɸú���ͳһ�ͷ�
 */
static const char* get_cookie(request_rec *r, const char* cookie_name)
{
    const char* cookie_header;
    char *value;

    // ������ͷ�еõ�
    // ap_table_get������strdup���ظ�cookie_header
    cookie_header = apr_table_get(r->headers_in, "Cookie");
    if(cookie_header != NULL)
    {
        if( (value = strstr(cookie_header, cookie_name)) != NULL )
        {
            char* cookie_buf;
            char* cookie_end;

            value += strlen(cookie_name) + 1; // ����һ��ð��
            cookie_buf = strdup(value);       // ���������ͷ�
            if( strcmp(cookie_name, "sign") != 0 )
            {
                cookie_end = strchr(cookie_buf, ',');
            }
            else
            {
                cookie_end = strchr(cookie_buf, '}');
            }

            if (cookie_end){
                *cookie_end = '\0';
			}

            return cookie_buf;
        }
    }

    return NULL;
}

/**
 * ��username timebefor notafter������д��JSON�ĸ�ʽ
 * ǩ��sign�ڴ˱�ѹ��
 */
static char* set_json_cookie_str(apr_pool_t* pool, char* username, char* timebefore, char* notafter, char* sign)
{
    char* compressed_signature = NULL;
    char* json_cookie = NULL;
    if(!pool || !username || !timebefore || !notafter || !sign){
        return NULL;
	}

    ////ѹ��ǩ��
    compressed_signature = compress_fn(sign);
    if(compressed_signature == NULL) {
		return NULL;
	}

    json_cookie = apr_pstrcat(pool,
                  "{username:", username, ",",
                   "timebefore:", timebefore, ","
                   "notonorafter:", notafter, ","
                   "sign:", compressed_signature, "}",
                    NULL);
    free(compressed_signature);

    return json_cookie;
}

/**
 * ��username timebefor notafter������д��JSON�ĸ�ʽ
 * ǩ��sign�ڴ˱�ѹ��
 */
static char* get_post_content(request_rec* r)
{
    int bytes = 0;
    int eos = 0;
    apr_size_t count = 0; // ��¼POST�������ݵ����ֽ���
    apr_status_t rv;
    apr_bucket_brigade* bb;
    apr_bucket_brigade* bbin;

    char* buf = NULL;
    apr_bucket* b;

    const char* clen = apr_table_get(r->headers_in, "Content-Length");
    if(clen != NULL){
        bytes = strtol(clen, NULL, 0);
	}
    else{
        bytes = MAX_SIZE;
	}

    bb   = apr_brigade_create(r->pool, r->connection->bucket_alloc);
    bbin = apr_brigade_create(r->pool, r->connection->bucket_alloc);

    do
    {
        rv = ap_get_brigade(r->input_filters, bbin, AP_MODE_READBYTES,APR_BLOCK_READ, bytes);
        if(rv != APR_SUCCESS) // ���ݽ��մ���
        {
              ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "[mod_passport] Failed to read from input!");
              return NULL;
        }

        for(b = APR_BRIGADE_FIRST(bbin);b != APR_BRIGADE_SENTINEL(bbin);)
        {
			if(APR_BUCKET_IS_EOS(b)) {
				eos = 1;
			}

			if(!APR_BUCKET_IS_METADATA(b))
			{
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

    buf = apr_palloc(r->pool, count + 1);
    apr_brigade_flatten(bb, buf, &count);
    buf[count] = '\0';

    return buf;
}

/**
 *  ɾ���ַ����е����л��з��ͻس���
 */
static void remove_lineends(char* buf)
{
    char* p1 = buf;
    char* p2 = buf;

    if(buf == NULL) {
		return;
	}

    while(1)
    {
        while(*p2 == '\r' || *p2 == '\n'){
            p2++;
		}
        *p1 = *p2;
		p1++;
        p2++;
		if(*p2 == '\0') {
			break;
		}
    }
    *p1='\0';
}

/**
 *  ����SAML�����ݣ�target�ɲ���target_buf���
 */
static char*get_saml_from_post(apr_pool_t* pool, char* post_buf)
{
    char* buf_decoded  = NULL;
    const char* samlpre = "samlResponse=";
    char* p = NULL;

    ap_unescape_url(post_buf); // URL����
    p = strstr(post_buf, samlpre);
    if(p == NULL) {
		return NULL;
	}
    p += strlen(samlpre);
    remove_lineends(p); // ��Base64����֮ǰ��ɾ�����з�
    buf_decoded = ap_pbase64decode(pool, p); // Base64����

    return buf_decoded;
}

/**
 * ��֤POST�������������ǲ��Ǵ�IDP������
 * �ǣ�����1
 * �񣬷���0
 */
static int verify_saml_response(request_rec* r, const char* responsexml)
{
   const char* certificate = NULL;
   const char* schema      = NULL;

   certificate = get_certificate_addr(r);
   schema = get_saml_schema_addr(r);
   
   if(!certificate || !schema) 
   {
       log_error(r,"[mod_passport] Cannot find certificate or schema!");
       return 0;
   }

   return verify_fn(responsexml, certificate, schema);
}

/**
 * ����Cookie������
 * Cookie���Ե�ǰ�Ự��Ч 
 */
static int set_cookie(request_rec* r, char* cookie_content)
{
   char* new_cookie = NULL;
   const char* path = NULL;

   passport_dir_conf* dcfg = (passport_dir_conf*)ap_get_module_config(r->per_dir_config,&passport_module);
   if(dcfg->address == NULL) {
       path = "/";
	}
   else{
       path = dcfg->address;
	}

   new_cookie = apr_pstrcat(r->pool,
                   "jpassport-sp=", cookie_content, ";"
                   "path=", path, ";"
                   /*"expires=Wednesday, 19-OCT-13 23:12:40 GMT;"*/, // ���Ե�ǰ�Ự��Ч
                   NULL);

   apr_table_setn(r->err_headers_out, "Set-Cookie", new_cookie);

   return 0;
}

/**
 * ͳһ�ͷ�Cookie�Ĳ���
 */
static int free_cookie_param(const char* username,const char* t1,const char* t2,const char* sign)
{
    free((void*)username);
    free((void*)t1);
    free((void*)t2);
    free((void*)sign);

    return 0;
}

/**
 * �ж�POST�����Ŀ���ǲ���shire
 */
static int is_post_shire(request_rec* r)
{
    char* p = NULL;

    p = strstr(r->uri, "shire");

    if(!p) {
		return 0;
	}

    return 1;
}

/**
 * �õ��û������URL����target
 * �û�����ΪPOSTʱר��
 */
static char* get_target_from_post(apr_pool_t* pool, char* post_buf)
{
    char* needle = "target=";
    char* p = NULL;
    char* q = NULL;
    char* target = NULL;
    int i = 0;

    p = strstr(post_buf, needle);
    if(p == NULL) {
		return NULL;
	}
    p += strlen(needle);

    q = strstr(post_buf, "&");
    if(q == NULL) {
		return NULL;
	}

    target = (char*)apr_palloc(pool, (q - p + 1) * sizeof(char));

    while(*p != *q)
    {
      target[i] = *p;
      i++;
      p++;
    }
    target[i] = '\0';

    ap_unescape_url(target);

    return target;
}

/**
 * �õ��û������URL����target
 * �û�����ΪGETʱר��
 */
static char* get_target_from_get(request_rec* r)
{
    char strport[10];
    unsigned int port;
    char* target = NULL;
    port = ap_get_server_port(r);
    sprintf(strport, ":%d", port);

    target = apr_pstrcat(r->pool, "http://", r->hostname, strport, r->uri, NULL);

    if(r->args != NULL){
		target = apr_pstrcat(r->pool, target, "?", r->args, NULL);
	}

    return target;
}

/**
 * �õ��ض���IDPʱ��URL
 * GETʱר��
 */
static char* get_redirect_url(request_rec* r, const char* target, const char* shire)
{
    return apr_pstrcat(r->pool,get_idp_addr(r),"?shire=", shire, "&target=", target,NULL);
}

/**
 * ������ָ��õ�resource��action����
 */
static int get_resource_action(request_rec* r, char* target, char** resource, char** action)
{
   int i;
   const char* s = NULL;
   char* p1 = NULL;
   char* p2 = NULL;
   char* p3 = NULL;

   apr_array_header_t* filter_array = get_filter_array(r);
   if(filter_array == NULL)
   {
        *resource = NULL;
        *action   = NULL;
        return -1;
   }

   for (i = 0; i < filter_array->nelts; i++)
   {
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
        if(ap_regexec(regexp, target, 0, NULL, 0) == 0)
        {
            break;
        }
   }

   if(p2 != NULL)
   {
       *resource = apr_pstrdup(r->pool, p2 + 1);
       p3 = strstr(*resource, " ");
       if(p3 != NULL)
       {
           *p3 = '\0';
           *action = p3 + 1;
       }
       else{
           *action = "access"; // Ĭ��
		}
   }
   else
   {
       *resource = NULL;
       *action   = NULL;
   }
   return 0;
}

/**
 *  JPassport-Client-Apache-Module�����幦�ܺ���
 *  ������������(handler)֮ǰ��Apache����
 */
static int fixups(request_rec* r)
{
    int enabled      = 0;
    const char* privatekey = NULL;
    const char* shire      = NULL;

    // �鿴JPassport Module�Ƿ񱻿���
    enabled = is_passport_enabled(r);
    // �������ã����mod_passport��ΪOFF��ֱ�ӷŹ�
    if(enabled == 0)
    {
        return DECLINED;
    }

    // ��������ļ����Ͳ��ڹ��˷�Χ����ֱ�ӷŹ�
    if(filter(r) == 0)
    {
        return DECLINED;
    }

    shire = get_shire_addr(r);
    privatekey = get_private_key_addr(r);

    if(!shire || !privatekey){
      return HTTP_INTERNAL_SERVER_ERROR;
	}

    if(r->method_number == M_POST && is_post_shire(r))
    {
        char* buf_decoded = NULL;   // samlResponse����������
        char* post_buf    = NULL;   // �����POST��������
        char* target      = NULL;   // �û�������

        // ��ȡ�ͻ���POST��������
        post_buf = get_post_content(r);

        if(post_buf != NULL)
        {
           int rv = 0;
           target = get_target_from_post(r->pool, post_buf);

           // �õ�POST���������е�SAML
           buf_decoded = get_saml_from_post(r->pool, post_buf); // buf_decode����r->pool�з����
           rv = verify_saml_response(r, buf_decoded);

           // У��
           if(rv == 1) //�ɹ�
           {
                char* cookie    = NULL;
                char* username  = NULL;
                char* notbefore = NULL;
                char* notafter  = NULL;
                char* signvalue = NULL;

                //// ׼��Cookie�ĸ������
                cookie_param_fn(buf_decoded, 
                                &username, &notbefore, &notafter, &signvalue,
                                privatekey,
                                get_private_key_password(r));

                cookie = set_json_cookie_str(r->pool, username, notbefore, notafter, signvalue);
                if(-1 == set_cookie(r, cookie)) return HTTP_INTERNAL_SERVER_ERROR;

                apr_table_set(r->err_headers_out, "Http", "302");
                apr_table_set(r->err_headers_out, "Location", target);

                //// ͨ����֤���趨�û�������֤����
                r->user = apr_pstrdup(r->pool, username);
                if(is_shortusername(r))
                {
                   char* p = strstr(r->user, "@");
                   if(p != NULL) *p = '\0';
                }

                r->ap_auth_type = apr_pstrdup(r->pool, "JPassport");
                //�ͷ�Cookie����
                free_saml_cookie_paramter_fn(username, notbefore, notafter, signvalue);

                return HTTP_MOVED_TEMPORARILY;
           }
           else if(rv == 0) // ��֤ʧ��
           {
                log_error(r, "[mod_passport] Verify Failed!");
                //// �ض���������ҳ��
                return HTTP_FORBIDDEN;
           }
        } 
        else
        {
           log_error(r, "[mod_passport] Failed to read from input");
           return HTTP_INTERNAL_SERVER_ERROR;
        }
    }
    else if(!is_post_shire(r))
    {
        const char* username    = NULL;
        const char* notbefore   = NULL;
        const char* notafter    = NULL;
        const char* signvalue   = NULL;
        const char* signcompare = NULL;
        char*       target      = NULL;
        char*       strsum      = NULL;
        char*       action      = NULL;
        char*       resource    = NULL;
        int         decision    = 0;
        const char* uncompressed_signvalue = NULL;

        // �õ��û��������ҳ��URL
        target = get_target_from_get(r);
        username = get_cookie(r, "username");

        // ����Ƿ���֮ǰ��¼ʱд��cookie
        // ���û�����ض�������¼ҳ��
	if(username  == NULL)
        {
            const char* redirect_url = NULL;

            redirect_url = get_redirect_url(r, target, shire);
            set_redirect(r, redirect_url);

            return HTTP_MOVED_TEMPORARILY;
        }

        //// ��֤Cookie����Ч��
        notbefore = get_cookie(r, "timebefore");
        notafter  = get_cookie(r, "notonorafter");
        signvalue = get_cookie(r, "sign");

        // ��ѹ��ǩ��
        uncompressed_signvalue = uncompress_fn(signvalue);
        strsum = apr_palloc(r->pool, strlen(username) + strlen(notbefore) + strlen(notafter) + 1);

        strsum[0] = '\0';

        strcat(strsum, username);
        strcat(strsum, notbefore);
        strcat(strsum, notafter);

        //// ��֤ǩ���ĺϷ���
        signcompare = sign_fn(strsum, privatekey, get_private_key_password(r));
        if(strcmp(uncompressed_signvalue, signcompare) != 0)
        {
           log_error(r, "[mod_passport] Signature not match!");

           free((void*)uncompressed_signvalue);
           free_cookie_param(username, notbefore, notafter, signvalue);

           return HTTP_FORBIDDEN; 
        }

        //// ͨ����֤��ȷ���û�������֤���͡�Apache2.xר��
        r->user = apr_pstrdup(r->pool, username);
        if(is_shortusername(r))
        {
            char* p = strstr(r->user, "@");
            if(p != NULL) {
				*p = '\0';
			}
        }

        r->ap_auth_type = apr_pstrdup(r->pool, "JPassport");

        //// ȷ���û���Ȩ��
        get_resource_action(r, r->uri, &resource, &action);

        //// �ж�Ȩ��
        decision = get_decision_fn(username, resource, action, get_idp_auth_hostname(r), get_idp_auth_path(r), get_idp_https_port(r));
        if(decision != 1)
        {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_passport] %s has been hold up(no authentication)!", username);
            // �ͷŽ�ѹ���ǩ��
            free((void*)uncompressed_signvalue);
            // �ͷ�Cookie����
            free_cookie_param(username, notbefore, notafter, signvalue);
            return HTTP_FORBIDDEN;
        }

        // �ͷŽ�ѹ���ǩ��
        free((void*)uncompressed_signvalue);
        // �ͷŸ���Cookie����
        free_cookie_param(username, notbefore, notafter, signvalue);
    }
    else
    {
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    return DECLINED;
}

static void dso_cleanup (void *d)
{
    //��������libpassport.library 
    if(dso_h != NULL)
    {
       apr_dso_unload(dso_h);
       dso_h = NULL;
    }
}

static int flag = 0;
/**
 *  ������Ҫ�õ���DSO
 *  ���еĺ���ֻ����һ��
 */
static void init_Child(apr_pool_t *p, server_rec *s)
{
    if(flag != 0) // Ϊ�˽�ʡʱ�䣬��Ҫ�ĺ���ֻ�ܵ���һ��
    {
		return;
	}
	apr_status_t rv    = 0;
	apr_pool_t*  mp    = NULL;
	const char*  fname = NULL;

	fname = get_libpassport_addr(s);

	apr_initialize();
	apr_pool_create(&mp, p);

	//// ����libpassport.so
	if ((rv = apr_dso_load(&dso_h, fname, mp)) != APR_SUCCESS)
	{
		ap_log_perror(APLOG_MARK, APLOG_ERR, 0, mp, "[mod_passport] Load libpassport error!");
		return;
	}

	//// ����libpassport.so�еĺ���
	if ((rv = apr_dso_sym((apr_dso_handle_sym_t*)&verify_fn, dso_h, "verify_saml_response")) != APR_SUCCESS)
	{
		ap_log_perror(APLOG_MARK, APLOG_ERR, 0, mp, "[mod_passport] Seek function verify from libpassport error!");
		return;
	}

	if ((rv = apr_dso_sym((apr_dso_handle_sym_t*)&cookie_param_fn, dso_h, "get_saml_cookie_parameter")) != APR_SUCCESS)
	{
		ap_log_perror(APLOG_MARK, APLOG_ERR, 0, mp,"[mod_passport] Seek function cookie_param from libpassport error!");
		return;
	}

	if ((rv = apr_dso_sym((apr_dso_handle_sym_t*)&sign_fn, dso_h, "sign")) != APR_SUCCESS)
	{
		ap_log_perror(APLOG_MARK, APLOG_ERR, 0, mp, "[mod_passport] Seek function sign from libpassport error!");
		return;
	}

	if ((rv = apr_dso_sym((apr_dso_handle_sym_t*)&free_saml_cookie_paramter_fn, dso_h, "free_saml_cookie_paramter")) != APR_SUCCESS)
	{
		ap_log_perror(APLOG_MARK, APLOG_ERR, 0, mp,"[mod_passport] Seek function free_saml_cookie from libpassport error!");
		return;
	}
	if ((rv = apr_dso_sym((apr_dso_handle_sym_t*)&get_decision_fn, dso_h,"get_decision")) != APR_SUCCESS)
	{
		ap_log_perror(APLOG_MARK, APLOG_ERR, 0, mp, "[mod_passport] Seek function get_decision from libpassport error!");
		return;
	}

	if ((rv = apr_dso_sym((apr_dso_handle_sym_t*)&compress_fn, dso_h,"compress_cookie_signature")) != APR_SUCCESS)
	{
		ap_log_perror(APLOG_MARK, APLOG_ERR, 0, mp,"[mod_passport] Seek function compress_cookie_signature from libpassport error!");
		return;
	}

	if ((rv = apr_dso_sym((apr_dso_handle_sym_t*)&uncompress_fn, dso_h, "uncompress_cookie_signature")) != APR_SUCCESS)
	{
		ap_log_perror(APLOG_MARK, APLOG_ERR, 0, mp,"[mod_passport] Seek function uncompress_cookie_signature from libpassport error!");
		return;
	}


	if(verify_fn          != NULL
	   && cookie_param_fn != NULL
	   && sign_fn         != NULL
	   && get_decision_fn != NULL
	   && compress_fn     != NULL
	   && uncompress_fn   != NULL
	   && free_saml_cookie_paramter_fn != NULL){

		flag++; 
	}
	else
	{
		ap_log_perror(APLOG_MARK, APLOG_ERR, 0, mp, "[mod_passport] Not all funcitions in libpassport was loaded!");
	}

	// ע�����DSO�ĺ���
	apr_pool_cleanup_register(p, NULL, dso_cleanup, apr_pool_cleanup_null);
}

static const command_rec passport_cmds[] =
{
    //// ÿĿ¼����
    AP_INIT_FLAG("JPassportEnable", // �Ƿ���JPassport
                 mod_passport_set_passport_enable,
                 NULL,
                 ACCESS_CONF|RSRC_CONF,
                 "whether or not to enable passport"),

    AP_INIT_FLAG("JPassportShortUserName", // �Ƿ���JPassport
                 mod_passport_short_username,
                 NULL,
                 ACCESS_CONF|RSRC_CONF,
                 "whether or not to enable passport"),


    AP_INIT_TAKE1("JPassportRootPathURI",
                  mod_passport_set_passport_address,
                  NULL,
                  ACCESS_CONF|RSRC_CONF,
                  "where address to send"),

    AP_INIT_TAKE1("JPassportPrivateKeyFileName",
                  mod_passport_set_private_key_address,
                  NULL,
                  ACCESS_CONF|RSRC_CONF,
                  "Private Key address"),

    AP_INIT_TAKE1("JPassportShireURI",
                  mod_passport_set_shire_address,
                  NULL,
                  ACCESS_CONF|RSRC_CONF,
                  "Shire address"),

    AP_INIT_TAKE1("JPassportPrivateKeyPassword",
                  mod_passport_set_private_key_password,
                  NULL,
                  ACCESS_CONF|RSRC_CONF,
                  "Private Key password"),

    AP_INIT_TAKE1("JPassportFilter",
                  mod_passport_set_filter_string,
                  NULL,
                  ACCESS_CONF|RSRC_CONF,
                  "Filter string"),


    //// ÿ����������
    AP_INIT_TAKE1("JPassportIDPUrl",
                  mod_passport_set_idp_url_address,
                  NULL,
                  RSRC_CONF,
                  "IDP address"),

    AP_INIT_TAKE1("JPassportClientPath",
                  mod_passport_set_client_path,
                  NULL,
                  RSRC_CONF,
                  "Client Path"),

    AP_INIT_TAKE1("JPassportCertificateFileName",
                  mod_passport_set_certificate_filename,
                  NULL,
                  RSRC_CONF,
                  "Certificate filename"),

    AP_INIT_TAKE1("JPassportAuthenticationURL",
                  mod_passport_set_authentication_url,
                  NULL,
                  RSRC_CONF,
                  "IDP HTTPS port"),

    {NULL}
};

static void passport_register_hooks(apr_pool_t *p)
{
    ap_hook_child_init(init_Child, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_fixups(fixups, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA passport_module =
{
    STANDARD20_MODULE_STUFF,
    mod_passport_create_per_dir,
    mod_passport_merge_per_dir,
    mod_passport_create_per_server,
    mod_passport_merge_per_server,
    passport_cmds,
    passport_register_hooks
};
