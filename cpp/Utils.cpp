#include "Utils.h"

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
	int i = str.length()-1;
	while(list.find(str[i]) != string::npos){
		str.erase(i,1);
		i = str.length()-1;
	}
}

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