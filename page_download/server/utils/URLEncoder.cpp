#include "utils/URLEncoder.h"

#include <string>
#include <iostream>

//将url转换为application/x-www-form-urlencoded MIME格式
//转换算法参见：http://java.sun.com/j2se/1.4.2/docs/api/，java.net.URLEncoder类
//在标准编码转换函数上做了修改：':', '/'也保留原始符号

URLEncoder::URLEncoder(void)
{
}

URLEncoder::~URLEncoder(void)
{
}

//字节映射表，如果对应值不等于0，则转码为对应值；否则，转换为%XY，其中XY为字节的hex表示
static const unsigned char bytemap[0x100] =
{
	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0', 
	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0', 
	'+',	'\x21',	'\x22',	'\x23',	'\x24',	'\x25',	'\x26',	'\x27',	'\x28',	'\x29', '*',	'\x2B',	'\x2C',	'-',	'.',	'/', 
	'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',	'9',	':',	'\x3B',	'\x3C',	'\x3D',	'\x3E',	'\x3F', 
	'\x40',	'A',	'B',	'C',	'D',	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O', 
	 'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	'X',	'Y',	'Z',	'\x5B',	'\x5C', '\x5D',	'\x5E',	'_', 
	'\0',	'a',	'b',	'c',	'd',	'e',	'f',	'g',	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o', 
	'p',	'q',	'r',	's',	't',	'u',	 'v',	'w',	'x',	'y',	'z',	'\x7B',	'\x7C',	'\x7D',	'\x7E',	'\x7F', 
	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0', 
	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0', 
	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0', 
	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0', 
	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0', 
	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0', 
	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0', 
	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0',	'\0', 
};

static const char hexch[] = "0123456789ABCDEF";


//将长度为srclen的url串src编码转换为MIME格式，保存在长度为destsize的缓冲区dest中
//返回转换结果的长度，如果出错，返回-1
int URLEncoder::encoding(const char * src, int srclen, char * dest, int destsize)
{
	if ((src == NULL) || (srclen <= 0))
	{
		return 0;
	}

	const unsigned char * pscan = (const unsigned char *)src;
	const unsigned char * pend = pscan + srclen;
	unsigned char * pdest = (unsigned char *)dest;
	unsigned char * pdestend = pdest + destsize - 4;	//为最后一个字符预留4个字节

	while ((pscan < pend) && (pdest < pdestend))
	{
		if (bytemap[*pscan])
		{
			*pdest = bytemap[*pscan];
			pdest ++;
		}
		else
		{
			*pdest = '%';
			pdest ++;
			*pdest = hexch[*pscan >> 4];
			pdest ++;
			*pdest = hexch[*pscan & 0x0F];
			pdest ++;
		}
		pscan ++;
	}

	int destlen = (int)(pdest - (unsigned char *)dest);
	if (destlen >= 0)
	{
		dest[destlen] = '\0';
	}
	return destlen;
}

//将以'\0'结尾的url串src编码转换为MIME格式，保存在长度为destsize的缓冲区dest中
//返回转换结果的长度，如果出错，返回-1
int URLEncoder::encoding(const char * src, char * dest, int destsize)
{
	if (src == NULL)
	{
		return 0;
	}

	const unsigned char * pscan = (const unsigned char *)src;
	unsigned char * pdest = (unsigned char *)dest;
	unsigned char * pdestend = pdest + destsize - 4;	//为最后一个字符预留4个字节

	while ((*pscan != '\0') && (pdest < pdestend))
	{
		if (bytemap[*pscan])
		{
			*pdest = bytemap[*pscan];
			pdest ++;
		}
		else
		{
			*pdest = '%';
			pdest ++;
			*pdest = hexch[*pscan >> 4];
			pdest ++;
			*pdest = hexch[*pscan & 0x0F];
			pdest ++;
		}
		pscan ++;
	}

	int destlen = (int)(pdest - (unsigned char *)dest);
	if (destlen >= 0)
	{
		dest[destlen] = '\0';
	}
	return destlen;
}

//将长度为srclen的url串按MIME格式解码，保存在长度为destsize的缓冲区dest中
//返回解码结果的长度，如果出错，返回-1
int URLEncoder::decoding(const char * src, int srclen, char * dest, int destsize)
{
	if ((src == NULL) || (srclen == 0))
	{
		return 0;
	}

	enum DECODER_STATE
	{
		DS_COMMON, DS_ESCAPE1, DS_ESCAPE2,
	} state = DS_COMMON;

	const unsigned char * pscan = (const unsigned char *)src;
	const unsigned char * pend = pscan + srclen;
	unsigned char * pdest = (unsigned char *)dest;
	unsigned char * pdestend = pdest + destsize - 4;	//为最后一个字符预留4个字节;

	char deescape;
	bool nonescback = false;
	bool ifargu = false;				//是否进入参数部分
	while ((pscan < pend) && (pdest < pdestend))
	{
		switch (state)
		{
		case DS_COMMON:
			{
				//非转义字符
				if (*pscan <= 0x20)
				{
					//东坡说0x20下所有的字符编码
					if ((*pscan == '\t') || (*pscan == '\r') || (*pscan == '\n'))		//冯刚说这3个字符过滤
					{
						pscan ++;
						break;
					}

					*pdest = '%';
					pdest ++;
					*pdest = hexch[*pscan >> 4];
					pdest ++;
					*pdest = hexch[*pscan & 0x0F];
					pdest ++;

					pscan ++;
					break;
				}

				if ((*pscan == ';') || (*pscan == '?'))
				{
					ifargu = true;
				}

				switch (*pscan)
				{
				/*
				case '+':
					{
						*pdest = ' ';
						pdest ++;
						break;
					}
				*/
				case '%':
					{
						if ((nonescback) || ((pscan + 2) >= pend))
						{
							//如果是发现"%XX"非转义回退
							*pdest = *pscan;
							nonescback = false;
							pdest ++;
						}
						else
						{
							state = DS_ESCAPE1;
						}
						break;
					}
				case '\\':
					{
						if (ifargu)
						{
							*pdest = '\\';			//参数中的'\\'不转换
						}
						else
						{
							*pdest = '/';			//将非参数'\\'转换为'/'
						}
						pdest ++;
						break;
					}
				default:
					{
						*pdest = *pscan;
						pdest ++;
						break;
					}
				}
				pscan ++;
				break;
			}
		case DS_ESCAPE1:
			{
				//转义字符第一个字节，必须'0'~'9'或'A'~'F'
				if ((*pscan >= 'A') && (*pscan <= 'F'))
				{
					deescape = (*pscan - 'A' + 10)<<4;
					state = DS_ESCAPE2;
					pscan ++;
					break;
				}
				if ((*pscan >= 'a') && (*pscan <= 'f'))
				{
					deescape = (*pscan - 'a' + 10)<<4;
					state = DS_ESCAPE2;
					pscan ++;
					break;
				}

				//if ((*pscan >= '0') && (*pscan <= '9'))
				//东坡说，%20以下字符不作解码
				if ((*pscan >= '3') && (*pscan <= '9'))
				{
					deescape = (*pscan - '0')<<4;
					state = DS_ESCAPE2;
					pscan ++;
					break;
				}

				if ((*pscan == '2') && ((pscan + 1) < pend) && (*(pscan + 1) != '0'))
				{
					//%20不做解码，%21以上解码
					deescape = (*pscan - '0')<<4;
					state = DS_ESCAPE2;
					pscan ++;
					break;
				}

				//否则，认为"%XX"不是转义字符
				nonescback = true;
				pscan --;
				state = DS_COMMON;

				break;
			}
		case DS_ESCAPE2:
			{
				//转义字符第二个字节，必须'0'~'9'或'A'~'F'
				if ((*pscan >= 'A') && (*pscan <= 'F'))
				{
					deescape |= (*pscan - 'A' + 10);
					*pdest = deescape;
					pdest ++;
					pscan ++;
					state = DS_COMMON;
					break;
				}
				if ((*pscan >= 'a') && (*pscan <= 'f'))
				{
					deescape |= (*pscan - 'a' + 10);
					*pdest = deescape;
					pdest ++;
					pscan ++;
					state = DS_COMMON;
					break;
				}

				if ((*pscan >= '0') && (*pscan <= '9'))
				{
					deescape |= (*pscan - '0');
					*pdest = deescape;
					pdest ++;
					pscan ++;
					state = DS_COMMON;
					break;
				}

				//否则，认为"%XX"不是转义字符
				nonescback = true;
				pscan -= 2;
				state = DS_COMMON;

				break;
			}
		default:
			{
//				cerr<<"Error: illegal state in URLEncoder::decoding()."<<endl;
				return -1;
			}
		}
	}

	int destlen = 0;
	if (pdest < pdestend)
	{
		*pdest = '\0';
		 destlen = (int)(pdest - (unsigned char *)dest);
	}
	else
	{
		return -1;
	}

	return destlen;

}

//将以'\0'结尾的url串src按MIME格式解码，保存在长度为destsize的缓冲区dest中
//返回解码结果的长度，如果出错，返回-1
int URLEncoder::decoding(const char * src, char * dest, int destsize)
{
	if (src == NULL)
	{
		return 0;
	}

	enum DECODER_STATE
	{
		DS_COMMON, DS_ESCAPE1, DS_ESCAPE2,
	} state = DS_COMMON;

	const unsigned char * pscan = (const unsigned char *)src;
	unsigned char * pdest = (unsigned char *)dest;
	unsigned char * pdestend = pdest + destsize - 4;	//为最后一个字符预留4个字节;

	char deescape;
	bool nonescback = false;
	bool ifargu = false;						//是否进入参数部分
	while ((*pscan != '\0') && (pdest < pdestend))
	{
		switch (state)
		{
		case DS_COMMON:
			{
				//非转义字符
				if (*pscan <= 0x20)
				{
					if ((*pscan == '\t') || (*pscan == '\r') || (*pscan == '\n'))		//冯刚说这3个字符过滤
					{
						pscan ++;
						break;
					}

					//东坡说0x20下所有的字符编码
					*pdest = '%';
					pdest ++;
					*pdest = hexch[*pscan >> 4];
					pdest ++;
					*pdest = hexch[*pscan & 0x0F];
					pdest ++;

					pscan ++;
					break;
				}

				if ((*pscan == ';') || (*pscan == '?'))
				{
					ifargu = true;
				}

				switch (*pscan)
				{
				/*
				case '+':
					{
						*pdest = ' ';
						pdest ++;
						break;
					}
				*/
				case '%':
					{
						if ((nonescback) || (*(pscan + 1) == '\0') || (*(pscan + 2) == '\0'))
						{
							//如果是发现"%XX"非转义回退
							*pdest = *pscan;
							nonescback = false;
							pdest ++;
						}
						else
						{
							state = DS_ESCAPE1;
						}
						break;
					}
				case '\\':
					{
						if (ifargu)
						{
							*pdest = '\\';			//参数中的'\\'不转换
						}
						else
						{
							*pdest = '/';			//将非参数'\\'转换为'/'
						}
						pdest ++;
						break;
					}
				default:
					{
						*pdest = *pscan;
						pdest ++;
						break;
					}
				}
				pscan ++;
				break;
			}
		case DS_ESCAPE1:
			{
				//转义字符第一个字节，必须'0'~'9'或'A'~'F'
				if ((*pscan >= 'A') && (*pscan <= 'F'))
				{
					deescape = (*pscan - 'A' + 10)<<4;
					state = DS_ESCAPE2;
					pscan ++;
					break;
				}
				if ((*pscan >= 'a') && (*pscan <= 'f'))
				{
					deescape = (*pscan - 'a' + 10)<<4;
					state = DS_ESCAPE2;
					pscan ++;
					break;
				}

				//if ((*pscan >= '0') && (*pscan <= '9'))
				//东坡说，%20以下字符不作解码
				if ((*pscan >= '3') && (*pscan <= '9'))
				{
					deescape = (*pscan - '0')<<4;
					state = DS_ESCAPE2;
					pscan ++;
					break;
				}

				if ((*pscan == '2') && (*(pscan + 1) != '\0') && (*(pscan + 1) != '0'))
				{
					//%20不做解码，%21以上解码
					deescape = (*pscan - '0')<<4;
					state = DS_ESCAPE2;
					pscan ++;
					break;
				}


				//否则，认为"%XX"不是转义字符
				nonescback = true;
				pscan --;
				state = DS_COMMON;

				break;
			}
		case DS_ESCAPE2:
			{
				//转义字符第二个字节，必须'0'~'9'或'A'~'F'
				if ((*pscan >= 'A') && (*pscan <= 'F'))
				{
					deescape |= (*pscan - 'A' + 10);
					*pdest = deescape;
					pdest ++;
					pscan ++;
					state = DS_COMMON;
					break;
				}
				if ((*pscan >= 'a') && (*pscan <= 'f'))
				{
					deescape |= (*pscan - 'a' + 10);
					*pdest = deescape;
					pdest ++;
					pscan ++;
					state = DS_COMMON;
					break;
				}

				if ((*pscan >= '0') && (*pscan <= '9'))
				{
					deescape |= (*pscan - '0');
					*pdest = deescape;
					pdest ++;
					pscan ++;
					state = DS_COMMON;
					break;
				}

				//否则，认为"%XX"不是转义字符
				nonescback = true;
				pscan -= 2;
				state = DS_COMMON;

				break;
			}
		default:
			{
//				cerr<<"Error: illegal state in URLEncoder::decoding()."<<endl;
				return -1;
			}
		}
	}

	int destlen = 0;
	if (pdest < pdestend)
	{
		*pdest = '\0';
		 destlen = (int)(pdest - (unsigned char *)dest);
	}
	else
	{
		return -1;
	}

	return destlen;

}


