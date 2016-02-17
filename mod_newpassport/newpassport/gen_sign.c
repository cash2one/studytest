#include "gen_sign.h"
#include "config_parser.h"
#include "mod_newpassport.h"

/**
 *  导入签名时所需要的私匙
 */
int load_private_key(request_rec *r, const char* filename, const char* password, EVP_PKEY** pkey)
{
    BIO* bp;
    // 初始化 BIO
    if((bp = BIO_new(BIO_s_file())) == NULL)
        return -1;

    // 设定要读取的私匙位置
    BIO_read_filename(bp, filename);

    // 导入私匙
    *pkey = PEM_read_bio_PrivateKey(bp, NULL, NULL, (char *) password);
    if(*pkey == NULL){
        BIO_free(bp);
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_newpassport] get private key failed when sign cookie\n");
        return -1;
    }

    BIO_free(bp);

    return 0;
}

/**
 * 签名函数
 * 注意！返回的签名字符串需要用free来释放
 * msg: 用来签名的数据
 * private_key_addr: 私匙存放的位置
 */
int sign(request_rec *r, char* msg, char** sign_str)
{
    passport_server_conf* servconf = NULL;
    get_server_conf(r, &servconf);
    const char* private_key_addr = servconf->private_key_addr;
    const char* password = servconf->private_key_password;
    if(access(private_key_addr, R_OK) != 0){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "[mod_newpassport] read private_key_addr failed\n");
        return -1;
    }

    EVP_MD_CTX sign_ctx;
    EVP_PKEY* privKey = NULL;

    /* 存放簽名處，至少要和 private key 一樣長 */
    unsigned char sign_value[1024];
    unsigned int sign_len;

    OpenSSL_add_all_algorithms();
    int pkRet = load_private_key(r, private_key_addr, password, &privKey);
    if(0 != pkRet){
        ap_log_rerror(APLOG_MARK, APLOG_ERR, pkRet, r, "[mod_newpassport] unable to load private key\n");
        EVP_MD_CTX_cleanup(&sign_ctx);
        return -1;
    }
    EVP_SignInit(&sign_ctx, EVP_sha1());
    EVP_SignUpdate(&sign_ctx, msg, strlen(msg));
    EVP_SignFinal(&sign_ctx, sign_value, &sign_len, privKey);

    int i;
    char tmp_sign[1024] = {0};
    for(i = 0; i < sign_len; i++){
        sprintf(tmp_sign + i * 2, "%02X", sign_value[i]);
    }

    *sign_str = (char*) apr_pstrdup(r->pool, tmp_sign);
    EVP_PKEY_free(privKey);
    EVP_MD_CTX_cleanup(&sign_ctx);

    return 0;
}
