#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>
#include <apr_base64.h>

/**
 *  dest��Base64�����
 */
int compress_2_base64(unsigned char* source, unsigned long* destlen, char* dest)
{
    int rv = 0;
    int base64len = 0;
    unsigned char* temp_dest = NULL; // ����ѹ����Ķ���������

    temp_dest = (unsigned char*)malloc( (*destlen) * sizeof(unsigned char));

    //// ѹ�� 
    rv = compress(temp_dest, destlen, source, strlen((char*)source));

    //// Base64����
    base64len = apr_base64_encode_binary(dest, temp_dest, *destlen);

    *destlen = base64len;

    free(temp_dest);

    return rv;
}

/**
 *  source:    Base64������ַ���
 *  destlen:   ��ѹ֮����ַ�������
 *  dest:      ��ѹ֮����ַ���ָ��
 *  sourcelen: Base64�����ַ����ĳ���
 */
int base64de_uncompress(char* source,unsigned long* destlen,unsigned char* dest, unsigned long  sourcelen)
{
    int rv  = 0;
    unsigned long  base64de_len = 0;
    unsigned char* base64de_buf = NULL;

    base64de_len = (unsigned long)apr_base64_decode_len(source);
    base64de_buf = (unsigned char*)malloc(base64de_len * sizeof(unsigned char));

    //// Base64����
    apr_base64_decode((char*)base64de_buf, source);

    //// ��ѹ
    rv = uncompress(dest, destlen, base64de_buf, base64de_len);

    free(base64de_buf);

    return rv;
}

/**************************************************************************
 * ����ӿ�
 * ����Cookie��ǩ����ѹ��
 */

char*
compress_cookie_signature(char* signature)
{
    int rv;
    unsigned long destlen = 0;
    char* dest = NULL; // ָ��Base64�����ѹ���������

    unsigned char* sig = (unsigned char*)signature;
    dest = (char*)malloc( strlen((char*)sig) * sizeof(unsigned char) );
    destlen = (unsigned long)strlen((char*)sig);

    rv = compress_2_base64(sig, &destlen, dest);
    if(rv == 0) {
		return dest;
	}

    return NULL;
}

char*
uncompress_cookie_signature(char* base64signcompressed)
{
    int rv;
    unsigned long destlen   = 0;
    unsigned long sourcelen = 0;

    unsigned char* uncompressed = NULL;
    sourcelen = strlen(base64signcompressed);
    destlen   = 3 * sourcelen;
    uncompressed = (unsigned char*)malloc(3 * sourcelen * sizeof(unsigned char));

    rv = base64de_uncompress(base64signcompressed, &destlen, uncompressed, sourcelen);
    uncompressed[destlen] = '\0';
    if(rv == 0){
		return (char*)uncompressed;
	}

    return NULL;
}

/**************************************************************************/
