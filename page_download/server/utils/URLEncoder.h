#ifndef URLENCODING_H_
#define URLENCODING_H_


//将url转换为application/x-www-form-urlencoded MIME格式
//转换算法参见：http://java.sun.com/j2se/1.4.2/docs/api/，java.net.URLEncoder类
class URLEncoder
{
public:
	URLEncoder(void);
	~URLEncoder(void);

public:
	//将长度为srclen的url串src编码转换为MIME格式，保存在长度为destsize的缓冲区dest中
	//返回转换结果的长度，如果出错，返回-1
	static int encoding(const char * src, int srclen, char * dest, int destsize);

	//将以'\0'结尾的url串src编码转换为MIME格式，保存在长度为destsize的缓冲区dest中
	//返回转换结果的长度，如果出错，返回-1
	static int encoding(const char * src, char * dest, int destsize);

	//将长度为srclen的url串按MIME格式解码，保存在长度为destsize的缓冲区dest中
	//返回解码结果的长度，如果出错，返回-1
	static int decoding(const char * src, int srclen, char * dest, int destsize);

	//将以'\0'结尾的url串src按MIME格式解码，保存在长度为destsize的缓冲区dest中
	//返回解码结果的长度，如果出错，返回-1
	static int decoding(const char * src, char * dest, int destsize);

};

#endif
