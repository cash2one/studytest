#ifndef _UTILS_H_
#define _UTILS_H_

#include<string>
#include<vector>
#include<map>
#include<set>

using namespace std;

class Utils{
public:
	//去掉左侧的字符或字符串
	static void ltrim(string& str, int ch);
	static void ltrim(string& str, const string& list);
	
	//去掉两边的字符或字符串
	static void trim(string& str, int ch);
	static void trim(string& str, const string& list);
	
	//去掉右边的字符或字符串
	static void rtrim(string& str, int ch);
	static void rtrim(string& str, const string& list);
	
	//将字符串切割成vector
	static void explode(const string& str, const string& delim, vector<string>& ret);
	
	//将map、set、vector按连接符拼成字符串（只考虑value）
	static void implode(const map<string,string>& obj, const string& join, string& ret);
	static void implode(const vector<string>& obj, const string& join, string& ret);
	static void implode(const set<string>& obj, const string& join, string& ret);
	
	//将map的first和second提取成vector
	static void map_first(const map<string,string>& obj, vector<string>& ret);
	static void map_second(const map<string,string>& obj, vector<string>& ret);
};
#endif