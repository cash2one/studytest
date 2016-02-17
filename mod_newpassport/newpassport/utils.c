#include "utils.h"

/**
 * 对URL中出现的特殊字符进行URL Encoding
 */
void URLencode( char* str ){
	return;
}

/**
 *  删除字符串中的所有换行符和回车符
 */
void remove_lineends( char* buf ){
	char* p1 = buf;
	char* p2 = buf;

	if(buf == NULL) return;

	while(1){
		while(*p2 == '\r' || *p2 == '\n')
			p2++;
		*p1 = *p2;
		p1++;
		p2++;
		if(*p2 == '\0') break;
	}
	*p1 = '\0';
}