#include "compress.h"

/**
 *  dest是Base64编码的
 */
static int compress_2_base64(unsigned char* source, unsigned long* destlen, char* dest)
{
    int rv = 0;
    int base64len = 0;
    unsigned char* temp_dest = NULL; // 保存压缩后的二进制数据

    temp_dest = (unsigned char*) malloc((*destlen) * sizeof(unsigned char));

    //// 压缩 
    rv = compress(temp_dest, destlen, source, strlen((char*) source));

    //// Base64编码
    base64len = apr_base64_encode_binary(dest, temp_dest, *destlen);

    *destlen = base64len;

    free(temp_dest);

    return rv;
}

/**
 *  source:    Base64编码的字符串
 *  destlen:   解压之后的字符串长度
 *  dest:      解压之后的字符串指针
 *  sourcelen: Base64编码字符串的长度
 */
static int base64de_uncompress(char* source, unsigned long* destlen, unsigned char* dest, unsigned long sourcelen)
{
    int rv = 0;
    unsigned long base64de_len = 0;
    unsigned char* base64de_buf = NULL;

    base64de_len = (unsigned long) apr_base64_decode_len(source);
    base64de_buf = (unsigned char*) malloc(base64de_len * sizeof(unsigned char));

    //// Base64解码
    apr_base64_decode((char*) base64de_buf, source);

    //// 解压
    rv = uncompress(dest, destlen, base64de_buf, base64de_len);

    free(base64de_buf);

    return rv;
}

/**************************************************************************
 * 对外接口
 * 用于Cookie中签名的压缩
 */

int compress_cookie_signature(request_rec *r, char* signature, char** dest)
{
    int rv;
    unsigned long destlen = 0;
    //char* dest = NULL; // 指向Base64编码的压缩后的数据

    unsigned char* sig = (unsigned char*) signature;
    *dest = (char*) apr_pcalloc(r->pool, strlen((char*) sig) * sizeof(unsigned char));

    destlen = (unsigned long) strlen((char*) sig);

    rv = compress_2_base64(sig, &destlen, *dest);
    if(rv == 0){
        return 0;
    }

    ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "[mod_newpassport] compress_cookie_signature failed,signature=%s\n", (char*) signature);
    return -1;
}

int uncompress_cookie_signature(request_rec *r, char* base64signcompressed, char** uncompressed_signvalue)
{
    int rv;
    unsigned long destlen = 0;
    unsigned long sourcelen = 0;

    sourcelen = strlen(base64signcompressed);
    destlen = 3 * sourcelen;
    *uncompressed_signvalue = (unsigned char*) apr_pcalloc(r->pool, 3 * sourcelen * sizeof(unsigned char));

    rv = base64de_uncompress(base64signcompressed, &destlen, *uncompressed_signvalue, sourcelen);
    if(rv == 0){
        return 0;
    }

    ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "[mod_newpassport] uncompress signature failed failed,base64signcompressed=%s\n", (char*) base64signcompressed);
    return -1;
}
