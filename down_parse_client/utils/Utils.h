#ifndef _UTILS_H_
#define _UTILS_H_
#include<unistd.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdint.h>
#include<string>
#include<vector>
#include<map>
#include<set>
#include <uuid/uuid.h>



class Utils{
public:
	//去掉左侧的字符或字符串
	static void ltrim(std::string& str, int ch);
	static void ltrim(std::string& str, const std::string& list);
	
	//去掉两边的字符或字符串
	static void trim(std::string& str, int ch);
	static void trim(std::string& str, const std::string& list);
	
	//去掉右边的字符或字符串
	static void rtrim(std::string& str, int ch);
	static void rtrim(std::string& str, const std::string& list);
	
	//将字符串切割成std::vector
	static void explode(const std::string& str, const std::string& delim, std::vector<std::string>& ret);
	
	//检查文件所在的目录是否存在，否则就创建之，参数是文件
	static int32_t mk_sure_dir(const std::string& f);
	
	//产生唯一的uuid，返回字符串格式
	static void gen_uuid(std::string& ret);
	
	//将std::map、set、std::vector按连接符拼成字符串（只考虑value）
	static void implode(const std::map<std::string,std::string>& obj, const std::string& join, std::string& ret);
	static void implode(const std::vector<std::string>& obj, const std::string& join, std::string& ret);
	static void implode(const std::set<std::string>& obj, const std::string& join, std::string& ret);
	
	//将std::map的first和second提取成std::vector
	static void map_first(const std::map<std::string,std::string>& obj, std::vector<std::string>& ret);
	static void map_second(const std::map<std::string,std::string>& obj, std::vector<std::string>& ret);
	
	
	static unsigned char ToHex(unsigned char x);
	static unsigned char FromHex(unsigned char x);
	static void UrlEncode(const std::string& str, std::string& strTemp);
	static void UrlDecode(const std::string& str, std::string& strTemp);
};
#endif



