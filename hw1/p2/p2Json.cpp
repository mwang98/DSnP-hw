/****************************************************************************
  FileName     [ p2Json.cpp ]
  PackageName  [ p2 ]
  Synopsis     [ Define member functions of class Json and JsonElem ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2018-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>
#include "p2Json.h"

using namespace std;
/******************************************************************************
Public member function of Json.
******************************************************************************/
bool Json::read(const string& jsonFile){
	vector<string> info;
	vector<string>:: iterator it_str;
	fstream file(jsonFile, ios::in);

	if(!file.is_open())
		return false;

	while(file.peek() != EOF){
		string tmp;
		file >> tmp;
		info.push_back(tmp);
	}
	file.close(); 

	//clear unwanted info
	it_str = info.begin();
	while(it_str != info.end()){
		if(*it_str == "{" or *it_str == "}" or *it_str == ":" or *it_str == "," or *it_str == "" or *it_str == " ")
			info.erase(it_str);
		else if((*it_str).compare(0,1,"\"") == 0){
			(*it_str).erase((*it_str).begin());
			(*it_str).erase((*it_str).end()-1);
			it_str++;
		}
		else if((*it_str).compare(((*it_str).length())-1,1,",") == 0){
			(*it_str).erase((*it_str).end()-1);
			it_str++;
		}
		else
			it_str++;
	}

	//convert data into jsonElem
	it_str = info.begin();
	while(it_str != info.end()){
		add(*it_str, stoi(*(it_str+1)));
		it_str += 2 ;
	}

	return true;
}
void Json::print(){
	it = _obj.begin();

	cout << "{\n";
	if(!_obj.empty()){
		while(it != _obj.end()-1)
			cout << "  " << *(it++) << ",\n";
		cout << "  " << *it << endl;
	}
	cout << "}\n";
}
int Json::sum(){
	it = _obj.begin();
	int sum = 0;

	while(it != _obj.end())
		sum += (*(it++)).getValue();

	return sum;
}
double Json::ave(){
	return double(sum())/_obj.size();
}
JsonElem Json::max(){
	int max = -2147483648, pos=0, cnt=0 ;
	it = _obj.begin();
	while(it != _obj.end()){
		if((*it).getValue() > max){
			max = (*it).getValue();
			pos = cnt;
		}
		cnt++;
		it++;
	}
	return _obj.at(pos);
}
JsonElem Json::min(){
	int min = 2147483647, pos=0, cnt=0 ;
	it = _obj.begin();
	while(it != _obj.end()){
		if((*it).getValue() < min){
			min = (*it).getValue();
			pos = cnt;
		}
		cnt++;
		it++;
	}
	return _obj.at(pos);
}
void Json::add(const string& str, const int& val){
	JsonElem tmp(str,val);
	_obj.push_back(tmp);
}
bool Json::is_empty(){
	if(_obj.empty()){
		cout << "Error: No element found!!\n";
		return true;
	}
	return false;
}

/******************************************************************************
Friend function of JsonElem.
******************************************************************************/
ostream& operator << (ostream& os, const JsonElem& j){
	return (os << "\"" << j._key << "\" : " << j._value);
}





