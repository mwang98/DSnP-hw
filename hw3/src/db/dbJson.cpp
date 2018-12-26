/****************************************************************************
  FileName     [ dbJson.cpp ]
  PackageName  [ db ]
  Synopsis     [ Define database Json member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <climits>
#include <cmath>
#include <string>
#include <algorithm>
#include "dbJson.h"
#include "util.h"

using namespace std;

/*****************************************/
/*          Global Functions             */
/*****************************************/
ostream& operator << (ostream& os, const DBJsonElem& j){
   os << "\"" << j._key << "\" : " << j._value;
   return os;
}

istream& operator >> (istream& is, DBJson& j){
   // TODO: to read in data from Json file and store them in a DB 
   // - You can assume the input file is with correct JSON file format
   // - NO NEED to handle error file format
   	assert(j._obj.empty());
	j._readFlag = true;
	
	vector<string> info;
	vector<string>:: iterator it_str;

	while(is.peek() != EOF){
		string tmp;
		is >> tmp;
		info.push_back(tmp);
	}

	//clear unwanted char
	it_str = info.begin();
	while(it_str != info.end()){
		if(*it_str == "{" or *it_str == "}" or *it_str == ":" or \
		   *it_str == "," or *it_str == "" or *it_str == " ")
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
		else it_str++;
	}

	//convert data into jsonElem
	it_str = info.begin();
	while(it_str != info.end()){
		j.add(DBJsonElem(*it_str, stoi(*(it_str+1))));
		it_str += 2 ;
	}
	
   	return is;
}

ostream& operator << (ostream& os, const DBJson& j){
	size_t i = 0;
	if(j.empty()){
		cout << "{\n}\n";
		cout << "Total JSON elements: " << j._obj.size() << endl;
		return os;
	}
   	cout << "{\n";
   	for(; i < j._obj.size()-1 ; i++) 
		cout << "  " << j._obj[i] << ",\n";
   	cout << "  " << j._obj[i] << "\n}\n";
	cout << "Total JSON elements: " << j._obj.size() << endl;
   	return os;
}

/**********************************************/
/*   Member Functions for class DBJsonElem    */
/**********************************************/
/*****************************************/
/*   Member Functions for class DBJson   */
/*****************************************/
void DBJson::reset(){ 
	_obj.clear(); 
	_keySet.clear();
	_sum = 0;
	_readFlag = false; 
}

// return false if key is repeated
bool DBJson::add(const DBJsonElem& elm){
	if(!_readFlag) return false;
	if(_keySet.count(elm.key()) != 0) return false;

	_keySet.insert(elm.key());
	_obj.push_back(elm);
	_sum += elm.value();
   	return true;
}

// return NAN if DBJson is empty
float DBJson::ave() const{
   if(_obj.size() == 0) return NAN;
   return (float)sum() / _obj.size();
}

// If DBJson is empty, set idx to size() and return INT_MIN
int DBJson::max(size_t& idx) const{
	if(_obj.empty()) idx = _obj.size();
    int maxN = INT_MIN;
	for(size_t i = 0 ; i < _obj.size() ; i++)
		if(_obj[i].value() > maxN){
			maxN = _obj[i].value();
			idx = i;
		}
   	return  maxN;
}

// If DBJson is empty, set idx to size() and return INT_MAX
int DBJson::min(size_t& idx) const{
	if(_obj.empty()) idx = _obj.size();
	int minN = INT_MAX;
	for(size_t i = 0 ; i < _obj.size() ; i++)
		if(_obj[i].value() < minN){
			minN = _obj[i].value();
			idx = i;
		}
	return  minN;
}

void DBJson::sort(const DBSortKey& s){
   // Sort the data according to the order of columns in 's'
   ::sort(_obj.begin(), _obj.end(), s);
}

void DBJson::sort(const DBSortValue& s){
   // Sort the data according to the order of columns in 's'
   ::sort(_obj.begin(), _obj.end(), s);
}

// return 0 if empty
int DBJson::sum() const{ return _sum; }

