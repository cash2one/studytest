#ifndef _UTILS_H_
#define _UTILS_H_

#include<string>

using namespace std;

class Utils{
public:
	static void ltrim(string& str, int ch);
	static void ltrim(string& str, const string& list);
	static void trim(string& str, int ch);
	static void trim(string& str, const string& list);
	static void rtrim(string& str, int ch);
	static void rtrim(string& str, const string& list);
};
#endif