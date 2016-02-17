#include "utils/Utils.h"
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
	string str = obj_str;
	if( str.length() <= 0){
		return;
	}
	
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



