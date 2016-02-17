#ifndef _PARSEHTML_H_
#define _PARSEHTML_H_

#include<string>
#include<vector>
#include<map>

using namespace std;

class parseHTML{
public:
	parseHTML(){}
	int setXpath(const vector<string>& xpath);
	int setPage(const string& page);
	int parse(map<string, map<string, string> >& ret);
	
private:
	vector<string> _xpath;
	string _page_file;
};
#endif

