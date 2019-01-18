/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include <algorithm>
#include <vector>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;
void show(size_t a){
	size_t i = 0;
	vector<bool> v;
	while(i < 64){
		v.push_back(a % 2);
		a /= 2;
		++i;
	}
	for(size_t j = 0 ; j < v.size() ; ++j){
		if(j > 0 and j % 8 == 0) cout << "_";
		cout << v[v.size() -1 - j];
	}
}
/**************************************/
/*   class CirGate member functions   */
/**************************************/
unsigned CirGate::_globalRef = 1;

void CirGate::buildFanIn(vector<CirGate*>& list){
	/*
	for(size_t i = 0, n = _ip.size() ; i < n ; ++i){
		CirGate* tmp = list[_ip[i]/2];
		if(tmp == 0){
			tmp = new FloGate(_ip[i]/2);
			list[_ip[i]/2] = tmp;
		}
		GateV buff1(tmp,  _ip[i]%2);
		GateV buff2(this, _ip[i]%2);
		_fanIn.push_back  (buff1);
		tmp -> buildFanOut(buff2);
	}
	*/
}

void CirGate::buildFanIn(vector<CirGate*>& list, int ip1, int ip2){
	CirGate* tmp = list[ip1/2];
	if(tmp == 0)
		list[ip1/2] = tmp = new FloGate(ip1/2);
	GateV buff1(tmp,  ip1%2);
	GateV buff2(this, ip1%2);
	_fanIn.push_back  (buff1);
	tmp -> buildFanOut(buff2);

	if(ip2 == -1) return;
	tmp = list[ip2/2];
	if(tmp == 0)
		list[ip2/2] = new FloGate(ip2/2);
	
	GateV buff3(tmp,  ip2%2);
	GateV buff4(this, ip2%2);
	_fanIn.push_back  (buff3);
	tmp -> buildFanOut(buff4);
}
void CirGate::buildFanOut(const GateV& elm){ _fanOut.push_back(elm); }

void CirGate::reportGate() const{
	string out = "= " + getTypeStr() + "(" + to_string(getID()) + ")";
	if(!getSymbol().empty()) out += ("\"" + getSymbol() + "\"");
	out += (", line " + to_string(getLineNo()));
	cout << "================================================================================" << endl;
	cout << out << endl;
	cout << "= FECs:";	
	if(_fecPos != 0){
		unsigned literal = 0;
		for(size_t i = 0, n = (*_fecPos).size() ; i < n ; ++i) if((*_fecPos)[i]/2 == _id) literal = (*_fecPos)[i];
		for(size_t i = 0, n = (*_fecPos).size() ; i < n ; ++i){
			if((*_fecPos)[i]/2 == _id) continue;
			cout << " " <<((literal + (*_fecPos)[i]) % 2 == 0 ? "" : "!" ) << (*_fecPos)[i]/2;
		}
	}
	cout << endl;
	cout << "= Value: "; show(_value); cout << endl;
	cout << "================================================================================" << endl;
}
void CirGate::reportFanin (const int level, int nlv) const{
	if(nlv < 0) return;
	cout << getTypeStr() << " " << getID();
	// print (*) 需要：已尋訪、現在的層數並非最後一層（nlv = 0）、有_fanIn
	if(_ref >= _globalRef and nlv > 0 and !_fanIn.empty()){ cout << " (*)" << endl; return; }
	cout << endl;
	for(size_t i = 0 ; i < _fanIn.size() ; ++i){
		if(nlv > 0) for(int j = level - (nlv - 1) ; j > 0 ; --j) cout << "  ";
		if(_fanIn[i].is_invert() and nlv > 0) cout << "!";
		_fanIn[i].gatePtr() -> reportFanin(level, nlv-1);
	}
	nlv > 0 ? _ref++ : _ref;
}
void CirGate::reportFanout (const int level, int nlv) const{
	if(nlv < 0) return;
	cout << getTypeStr() << " " << getID();
	if(_ref >= _globalRef and nlv > 0 and !_fanOut.empty()){ cout << " (*)" << endl; return; }
	cout << endl;
	for(size_t i = 0 ; i < _fanOut.size() ; ++i){
		if(nlv > 0) for(int j = level - (nlv - 1) ; j > 0 ; --j) cout << "  ";	
		if(_fanOut[i].is_invert() and nlv > 0) cout << "!";
		_fanOut[i].gatePtr() -> reportFanout(level, nlv-1);
	}
	nlv > 0 ? _ref++ : _ref;
}
// ref 用在 DFS, Partail DFS traversal
void CirGate::resetToGlobalRef() const{
	if(_ref == _globalRef - 1) return;
	setToGlobalRef(); --_ref;

	for(size_t i = 0, n = _fanIn.size()  ; i < n ; ++i)
		_fanIn[i].gatePtr()  -> resetToGlobalRef();
	for(size_t i = 0, n = _fanOut.size() ; i < n ; ++i)
		_fanOut[i].gatePtr() -> resetToGlobalRef();
}
void CirGate::DFS(vector<unsigned>& DFSList, unordered_map<unsigned, unsigned>& map) const{
	if(_ref == _globalRef) return;

	for(size_t i = 0, n = _fanIn.size() ; i < n ; ++i)
		_fanIn[i].gatePtr() -> DFS(DFSList, map);
	DFSList.push_back(getID());
	map.insert(make_pair(getID(), map.size() - 1));
	setToGlobalRef();
}
/**************************************/
/*   class PiGate member functions   */
/**************************************/
void PiGate::printGate() const{
	cout << getTypeStr() << "  " << getID();
	if(getSymbol() != "") cout << " (" << getSymbol() << ")";
	cout << endl;
}
void PiGate::cmdWrite(ostream& os) const{
	os << getID()*2 << endl;
}
/**************************************/
/*   class ConstGate member functions */
/**************************************/
void ConstGate::printGate() const{
	cout << getTypeStr() << getID() << endl;
}
/**************************************/
/*   class FloGate member functions   */
/**************************************/
void FloGate::printGate() const{ return; }
/**************************************/
/*   class PoGate member functions   */
/**************************************/
void PoGate::printGate() const{
	cout << getTypeStr() << "  " << getID() << " ";
	if(_fanIn[0].gatePtr() -> getTypeStr() == "UNDEF") cout << "*";
	if(_fanIn[0].is_invert()) 						 cout << "!";
	cout << _fanIn[0].gatePtr() -> getID();
	if(getSymbol() != "") cout << " (" << getSymbol() << ")";
	cout << endl;
}
void PoGate::cmdWrite(ostream& os) const{
	for(size_t i = 0, n = _fanIn.size() ; i < n ; ++i)
		os << (_fanIn[i].gatePtr() -> getID())*2 + _fanIn[i].is_invert();
	os << endl;
}
void PoGate::simulation(){
	//_fanIn[0].gatePtr() -> simulation();
	if(_fanIn[0].is_invert())
		_value = ~(_fanIn[0].gatePtr() -> getValue());
	else
		_value =   _fanIn[0].gatePtr() -> getValue();
}
/**************************************/
/*   class AIGate member functions    */
/**************************************/
void AIGate::printGate() const{
	cout << getTypeStr() << " " << getID();
	for(size_t i = 0 ; i < _fanIn.size() ; ++i){
		cout << " ";
		if(_fanIn[i].gatePtr() -> getTypeStr() == "UNDEF") cout << "*";
		if(_fanIn[i].is_invert()) 						 cout << "!";
		cout << _fanIn[i].gatePtr() -> getID();
	}cout << endl;
}
void AIGate::cmdWrite(ostream& os) const{
	os << getID()*2;
	for(size_t i = 0, n = _fanIn.size() ; i < n ; ++i)
		os << " " << (_fanIn[i].gatePtr() -> getID())*2 + _fanIn[i].is_invert();
	os << endl;
}
void AIGate::simulation(){
	/*
	if(_ref == _globalRef) return;
	_fanIn[0].gatePtr() -> simulation();
	_fanIn[1].gatePtr() -> simulation();
	setToGlobalRef();
	*/
	_value 	= _fanIn[0].getSim() & _fanIn[1].getSim();
}