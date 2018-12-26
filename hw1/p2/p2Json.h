/****************************************************************************
  FileName     [ p2Json.h]
  PackageName  [ p2 ]
  Synopsis     [ Header file for class Json JsonElem ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2018-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef P2_JSON_H
#define P2_JSON_H

#include <vector>
#include <string>
#include <unordered_set>

using namespace std;

class JsonElem
{
public:
   // TODO: define constructor & member functions on your own
   JsonElem() {}
   JsonElem(const string& k, int v): _key(k), _value(v) {}

  friend ostream& operator << (ostream&, const JsonElem&);
  friend class Json;

private:
    string  _key;   // DO NOT change this definition. Use it to store key.
    int     _value; // DO NOT change this definition. Use it to store value.

    string getKey() const{ return _key; }
    int getValue() const{ return _value; }
};

class Json
{
public:
  Json(){}
  ~Json(){}
  bool read(const string&);
  void print();
  void add(const string&, const int&);
  int sum();
  double ave();
  JsonElem max();
  JsonElem min();
  bool is_empty();

private:
   vector<JsonElem>       _obj;  // DO NOT change this definition.
                                 // Use it to store JSON elements.
   vector<JsonElem>:: iterator it;
};

#endif // P2_TABLE_H
