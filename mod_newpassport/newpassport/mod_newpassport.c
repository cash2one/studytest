#include "mod_newpassport.h"
#include "compress.h"
#include "utils.h"
#include "config_parser.h"
#include "gen_sign.h"

/**
 * 处理passport.sogou-inc.com重定向来的POST请求，会POST过来两个参数：samlResponse和target
 */
int fixups_post_shire(request_rec *r)
{
    saml_elem* se = (saml_elem*) apr_pcalloc(r->pool, sizeof(saml_elem));

    ///获取POST过来的原始数据
    int postRet = get_post_content(r, se);
    if(0 != postRet || NULL == se->post_raw_data){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, postRet, r, "[mod_newpassport] get_post_content failed\n");
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    //要跳转的目标地址
    int targetRet = get_target_from_post(r->pool, se);
    if(0 != targetRet || se->target == NULL){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, targetRet, r, "[mod_newpassport] get_target_from_post failed\n");
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    //从POST里获取samlResponse字段的值，base64_decode后的
    int bufRet = get_saml_from_post(r->pool, se);
    if(0 != bufRet || se->saml_str == NULL){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, bufRet, r, "[mod_newpassport] get_saml_from_post failed\n");
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    //获取用于生成cookie信息的各个字段：username、notbefore、notafter、SignatureValue、DigestValue、sign
    int ret = get_saml_elem_value(r, se);
    if(0 != ret){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, ret, r, "[mod_newpassport] get_saml_elem_value failed\n");
        return HTTP_FORBIDDEN;
    }

    //拼接json形式的cookie值
    int ret_cookie = get_json_cookie_str(r, se);
    if(0 != ret_cookie){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, ret_cookie, r, "[mod_newpassport] get_json_cookie_str failed\n");
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    //开始种下一个cookie，下次登陆的时候会检测的
    int setCookieRet = set_cookie(r, se);
    if(0 != setCookieRet){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, setCookieRet, r, "[mod_newpassport] set_cookie failed\n");
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    //开始跳转页面，返回302响应码
    apr_table_setn(r->err_headers_out, "Http", "302");
    apr_table_setn(r->err_headers_out, "Location", se->target);

    //设置授权用户名及类型
    r->user = apr_pstrdup(r->pool, se->username);
    r->ap_auth_type = apr_pstrdup(r->pool, "JPassport");

    return HTTP_MOVED_TEMPORARILY;
}

/**
 * 处理首次访问时GET请求，验证cookie中的签名，若验证失败则重定向至登陆页面
 */
int fixups_get(request_rec *r)
{
    cookie_elem* ce = (cookie_elem*) apr_pcalloc(r->pool, sizeof(cookie_elem));

    ///获取cookie中的各个字段的值
    int nameRet = get_cookie(r, "username", &(ce->username));
    int beforeREt = get_cookie(r, "timebefore", &(ce->notbefore));
    int afterRet = get_cookie(r, "notonorafter", &(ce->notafter));
    int signRet = get_cookie(r, "sign", &(ce->cookie_sign));

    // 检查是否有之前登录时写的cookie，如果没有则重定向至登录页面
    if(nameRet != 0 || beforeREt != 0 || afterRet != 0 || signRet != 0 || ce->username == NULL || strlen(ce->username) <= 0){
        char* shire = NULL;
        char* target = NULL;
        char* redirect_url = NULL;
        get_shire_addr(r, &shire);
        get_target_from_get(r, &target);
        get_redirect_url(r, target, shire, &redirect_url);
        set_redirect(r, redirect_url);
        ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, "[mod_newpassport] HTTP_MOVED_TEMPORARILY\n");
        return HTTP_MOVED_TEMPORARILY;
    }


    // 解压缩签名：对比的就是压缩前的数据
    int uncomRet = uncompress_cookie_signature(r, ce->cookie_sign, &(ce->uncompressed_cookie_signvalue));
    if(0 != uncomRet){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, uncomRet, r, "[mod_newpassport] uncompress_cookie_signature failed\n");
        return HTTP_FORBIDDEN;
    }

    //分配空间，为重新签名做准备
    char* strsum = (char*) apr_pstrcat(r->pool, ce->username, ce->notbefore, ce->notafter, NULL);

    //// 对cookie中的字段重新签名
    int resignRet = sign(r, strsum, &(ce->regenerate_sign));
    if(0 != resignRet){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, resignRet, r, "[mod_newpassport] sign cookie info failed\n");
        return HTTP_FORBIDDEN;
    }

    //对比签名是否合法
    if(strcmp(ce->uncompressed_cookie_signvalue, ce->regenerate_sign) != 0){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_newpassport] Signature not match\n");
        return HTTP_FORBIDDEN;
    }

    //// 通过认证
    r->user = apr_pstrdup(r->pool, ce->username);
    r->ap_auth_type = apr_pstrdup(r->pool, "JPassport");

    return DECLINED;
}

/**
 *处理登陆请求的主函数，如果是未登陆的GET请求则调用fixups_get，
 * 若是从passport.sogou-inc.com重定向来的就走fixups_post_shire
 */
static int fixups(request_rec *r)
{
    //检测是否启用了本模块
    int enabled = is_passport_enabled(r);
    if(enabled == 0){
        ap_log_rerror(APLOG_MARK, APLOG_NOTICE, enabled, r, "[mod_newpassport] enabled=0\turi=%s\n", r->uri);
        return DECLINED;
    }

    //所请求的uri是否在过滤规则里
    if(is_filter(r) == 0){
        ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, "[mod_newpassport] is_filter=0\turi=%s\n", r->uri);
        return DECLINED;
    }

    //获取配置结构体指针
    passport_server_conf* servconf = NULL;
    get_server_conf(r, &servconf);
    if(NULL == servconf){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_newpassport] get_server_conf structure failed\n");
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    ///若没有私钥，就嘛也别想了
    const char* privatekey = servconf->private_key_addr;
    if(!privatekey){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_newpassport] canot find private_key_addr conf\n");
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    //开始处理请求
    int ret = DECLINED;
    r->user = NULL;
    if(r->method_number == M_POST && is_post_shire(r) == 1){
        ret = fixups_post_shire(r);
    }
    else if(is_post_shire(r) == 0){
        ret = fixups_get(r);
    }

    //is short username
    if(r->user){
        apr_table_setn(r->subprocess_env, "JPLoginName", apr_pstrdup(r->pool,r->user));
        if(is_passport_short_username(r) > 0){
            char* p = strstr(r->user, "@");
            if(NULL != p){
                *p = '\0';
            }
        }
    }

    return ret;
}

static void newpassport_register_hooks(apr_pool_t *p)
{
    ap_hook_fixups(fixups, NULL, NULL, APR_HOOK_MIDDLE);
}

static const command_rec newpassport_cmds[] = {
    AP_INIT_FLAG("newPassportEnable", mod_newpassport_set_passport_enable, NULL, ACCESS_CONF | RSRC_CONF, "whether or not to enable passport"),
    AP_INIT_TAKE1("newPassportPrivateKeyFileName", mod_newpassport_set_private_key_address, NULL, ACCESS_CONF | RSRC_CONF, "Private Key address"),
    AP_INIT_TAKE1("newPassportPrivateKeyPassword", mod_newpassport_set_private_key_password, NULL, ACCESS_CONF | RSRC_CONF, "Private Key password"),
    AP_INIT_TAKE1("newPassportIDPUrl", mod_newpassport_set_idp_url_address, NULL, ACCESS_CONF | RSRC_CONF, "IDP address"),
    AP_INIT_TAKE1("newPassportCertificateFileName", mod_newpassport_set_certificate_filename, NULL, ACCESS_CONF | RSRC_CONF, "Certificate filename"),
    AP_INIT_TAKE1("newPassportAuthenticationURL", mod_newpassport_set_authentication_url, NULL, ACCESS_CONF | RSRC_CONF, "IDP HTTPS port"),
    AP_INIT_FLAG("newPassportShortUserName", mod_newpassport_set_short_username, NULL, ACCESS_CONF | RSRC_CONF, "is permited short name"),
    AP_INIT_TAKE1("newPassportFilter", mod_newpassport_set_filter, NULL, ACCESS_CONF | RSRC_CONF, "filter url"),
    {NULL}
};
/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA newpassport_module = {
    STANDARD20_MODULE_STUFF,
    mod_newpassport_create_per_dir,
    mod_newpassport_merge_per_dir,
    mod_newpassport_create_per_server,
    mod_newpassport_merge_per_server,
    newpassport_cmds,
    newpassport_register_hooks
};

