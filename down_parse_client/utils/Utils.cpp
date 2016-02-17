#include "Utils.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <assert.h>

using namespace std;

///去掉字符串左侧的某字符或字符串
void Utils::ltrim(string& str, int ch){
    if(str.length() <= 0){
        return;
    }

	while(str[0] == ch){
		str.erase(0,1);
	}
}

void Utils::ltrim(string& str, const string& list){
	if(list.length() <= 0 || str.length() <= 0){
		return;
	}

	while(list.find(str[0]) != string::npos){
		str.erase(0,1);
	}
}

///去掉字符串两边的字符或字符串
void Utils::trim(string& str, int ch){
    if(str.length() <= 0){
        return;
    }

	while(str[0] == ch){
		str.erase(0,1);
	}
	int i = str.length()-1;
	while(str[i] == ch){
		str.erase(i,1);
		i = str.length()-1;
	}
}

void Utils::trim(string& str, const string& list){
	if(list.length() <= 0 || str.length() <= 0){
		return;
	}

	///遍历整个字符串
	while(list.find(str[0]) != string::npos){
		str.erase(0,1);
	}
	if( str.length() <= 0){
		return;
	}
	int i = str.length()-1;
	while(list.find(str[i]) != string::npos){
		str.erase(i,1);
		i = str.length()-1;
	}
}


///去掉字符串右侧的字符或字符串
void Utils::rtrim(string& str, int ch){
    if(str.length() <= 0){
        return;
    }

	int i = str.length()-1;
	while(str[i] == ch){
		str.erase(i,1);
		i = str.length()-1;
	}
}

void Utils::rtrim(string& str, const string& list){
	if(list.length() <= 0 || str.length() <= 0){
		return;
	}

	int i = str.length()-1;
	while(list.find(str[i]) != string::npos){
		str.erase(i,1);
		i = str.length()-1;
	}
}


//将字符串切割成vector
void Utils::explode(const string& obj_str, const string& delim, vector<string>& ret){
    if(obj_str.length() <= 0){
        return;
    }
	string str = obj_str;

	//delim为空
	if(delim.length() <= 0){
		for(int i=0; i < str.length(); i++){
			ret.push_back(string(1,str[i]));
		}
		return;
	}

	///不包含delim
	if( str.find(delim) == string::npos ){
		ret.push_back(str);
		return;
	}

	//包含的情况
	do{
		int pos = str.find(delim);
		string tmp = str.substr(0,pos);
		ret.push_back(tmp);
		pos = tmp.length() + delim.length();
		str = str.substr(pos, str.length()-pos);
	}while(str.find(delim) != string::npos);

	ret.push_back(str);
}

//检查文件的目录是否存在，否则就创建
int32_t Utils::mk_sure_dir(const string& f){
	if(access(f.c_str(),R_OK) == 0){
		return 0;
	}
	///切割文件路径，一段一段的检查
	vector<string> tmp_dir;
	explode(f,"/",tmp_dir);
	if(tmp_dir.size() <= 0){
		return -1;
	}

	string cur_dir = tmp_dir[0];
	for(vector<string>::size_type i=1; i < tmp_dir.size()-1; i++){
		cur_dir.append("/").append(tmp_dir[i]);
		if( access(cur_dir.c_str(),R_OK) == 0){
			continue;
		}
		if(mkdir(cur_dir.c_str(),0755) != 0){
			return -10;
		}
	}
	return 0;
}

//产生唯一的uuid，类似于：C126E1A3A3B2403EB57716E5935EA007
void Utils::gen_uuid(string& ret){
	uuid_t uu;
	char tmp[5] = {0};
	uuid_generate( uu );
    for(int i=0; i<16; i++)
    {
        sprintf(tmp, "%02X",uu[i]);
		ret.append(tmp);
		bzero(tmp,5);
    }
}

/**转主16进制**/
unsigned char Utils::ToHex(unsigned char x){
	return  x > 9 ? x + 55 : x + 48;
}

/**从16进制转回来**/
unsigned char Utils::FromHex(unsigned char x){
	unsigned char y;
	if (x >= 'A' && x <= 'Z'){
		y = x - 'A' + 10;
	}
	else if (x >= 'a' && x <= 'z'){ 
		y = x - 'a' + 10;
	}
	else if (x >= '0' && x <= '9'){
		y = x - '0';
	}
	else{
		assert(0);
	}
	return y;
}

/**对URL进行编码**/
void Utils::UrlEncode(const string& str, string& strTemp){
	size_t length = str.length();
	for (size_t i = 0; i < length; i++){
		if (isalnum((unsigned char)str[i]) || (str[i] == '-') || (str[i] == '_') ||(str[i] == '.') ||(str[i] == '~')){
			strTemp += str[i];
		}
		else if (str[i] == ' '){
			strTemp += "+";
		}
		else{
			strTemp += '%';
			strTemp += ToHex((unsigned char)str[i] >> 4);
			strTemp += ToHex((unsigned char)str[i] % 16);
		}
	}
}

/**对URL解码**/
void Utils::UrlDecode(const string& str, string& strTemp){
	size_t length = str.length();
	for (size_t i = 0; i < length; i++){
		if (str[i] == '+'){
			strTemp += ' ';
		}
		else if (str[i] == '%'){
			assert(i + 2 < length);
			unsigned char high = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += high*16 + low;
		}
		else{
			strTemp += str[i];
		}
	}
}

//将map、set、vector按连接符拼成字符串（只考虑value）
void Utils::implode(const map<string,string>& obj, const string& join, string& ret){

}
void Utils::implode(const vector<string>& obj, const string& join, string& ret){

}
void Utils::implode(const set<string>& obj, const string& join, string& ret){

}

//将map的first和second提取成vector
void Utils::map_first(const map<string,string>& obj, vector<string>& ret){

}
void Utils::map_second(const map<string,string>& obj, vector<string>& ret){

}



