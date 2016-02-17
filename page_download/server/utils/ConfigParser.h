#ifndef _CONFIGPARSER_H_
#define _CONFIGPARSER_H_

#include<map>
#include<string>

using namespace std;

class ConfigParser{
public:
	ConfigParser(const string& conf_file);
	ConfigParser();
	
	int get_item(const string& key, string& value);
	int set_config_file(const string& conf_file);
    int get_config_file(string& conf_file);
private:
	map<string,string> config;
	string config_file;
	int parse_conf();
};


#endif

