/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstdlib>
#include <queue>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}
/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
CirMgr::~CirMgr() {
	for(size_t i = 0, n = _GateList.size() ; i < n ; ++i)
		if(_GateList[i] != 0) delete _GateList[i];
} 
bool CirMgr::readCircuit(const string& fileName){
	ifstream file(fileName);
	string line;
	
	stringstream ss;
	regex title_pattern("^aag [0-9]+ [0-9]+ 0 [0-9]+ [0-9]+$");
	regex io_pattern   ("^[0-9]+$");
	regex aig_pattern  ("^[0-9]+ [0-9]+ [0-9]+$");
	regex sb_pattern   ("^[io][0-9]+ .+$");


	if(!file.is_open()){
      cerr << "Cannot open design \""<< fileName << "\"!!";
      return false;
   }
   getline(file, line); ss.str(line);
   if(regex_match(line, title_pattern)){
     	ss >> _header;
		for(unsigned i = 0 ; i < 5 ; ++i){
			string buff;
			ss >> buff;
			_miloa[i] = (unsigned)stoi(buff); // the order is : maxID, # of inputs, # of latches,
											//                # of outputs, # of AIGs
		}
		if(_miloa[0] < _miloa[1] + _miloa[4]){
			cerr << "[ERROR]" << endl;
			return false;
		}
   }
   else{
     	cerr << "[ERROR]" << endl;
   	return false;
   }

	_GateList.reserve(_miloa[0] + 1 + _miloa[3]);
	for(size_t i = 0 ; i < _miloa[0] + 1 + _miloa[3] ; ++i) _GateList.push_back(NULL);
	_GateList[0] = new ConstGate();

	size_t i = 0, lineNo = 2;
	//*******input*******//
	for(; i < _miloa[1] ; ++i, ++lineNo){
		unsigned id;
		getline(file, line);
		if(!regex_match(line, io_pattern)){
			cerr << "[ERROR]" << endl;
			return false;
		}
		ss.clear(); ss.str(line); ss >> id;

		CirGate* tmp = new PiGate(id/2, lineNo);
		_PiList.push_back(tmp -> getID());
		_GateList[tmp -> getID()]= tmp;
		++_totalGate;
	}
	//*******output*******//
	for(i = 0; i < _miloa[3] ; ++i, ++lineNo){
		unsigned ip, id = _miloa[0] + 1 + i;
		getline(file, line);
		if(!regex_match(line, io_pattern)){
			cerr << "[ERROR]" << endl;
			return false;
		}
		ss.clear(); ss.str(line); ss >> ip;
		
		//CirGate* tmp = new PoGate(id, lineNo, ip);
		CirGate* tmp = new PoGate(id, lineNo);
		_PoList.push_back(tmp -> getID());
		_GateList[tmp -> getID()]= tmp;
		++_totalGate;
	}
	for(i = 0 ; i < _miloa[4] ; ++i, ++lineNo){
		unsigned id, ip1, ip2;
		getline(file, line);
		if(!regex_match(line, aig_pattern)){
			cerr << "[ERROR]" << endl;
			return false;
		}
		ss.clear(); ss.str(line);
		ss >> id >> ip1 >> ip2;

		//CirGate* tmp = new AIGate(id/2, lineNo, ip1, ip2);
		CirGate* tmp = new AIGate(id/2, lineNo);
		_GateList[tmp -> getID()]= tmp;
		++_totalGate;
	}

   // Symbol section
	while(file.peek() != EOF){
		string tmp, sb;
     	getline(file, tmp);
		if(tmp == "c") break;
      if(!regex_match(tmp, sb_pattern)){
      	cerr << "[ERROR]" << endl;
      	return false;
      }
		char std = tmp[0];
		sb  = tmp.substr(tmp.find_first_of(" ") + 1); //eg. i36 $    _OUTPUT$ (symbol can contains whitespace)
		tmp = tmp.substr(1, tmp.find_first_of(" "));
		int pos = stoi(tmp);
		if(std == 'i')
			_GateList[_PiList[pos]] -> setSymbol(sb);
		else if(std == 'o')
			_GateList[_PoList[pos]] -> setSymbol(sb);
	}
	file.close();

	//After creating all gates, build connections afterward
	file.open(fileName);
	string gar;
	int outputNo = _miloa[0] + 1;
	getline(file,gar);
	for(unsigned j = 0, n = _miloa[1] + _miloa[3] + _miloa[4] ; j < n ; ++j){
		unsigned id, ip1, ip2;
		if(j < _miloa[1]) file >> id;
		else if(j < _miloa[1] + _miloa[3]){
			file >> ip1;
			assert(_GateList[outputNo] != 0);
			_GateList[outputNo++] -> buildFanIn(_GateList, ip1, -1);
		}
		else if(j < _miloa[1] + _miloa[3] + _miloa[4]){
			file >> id >> ip1 >> ip2;
			assert(_GateList[id/2] != 0);
			_GateList[id/2] -> buildFanIn(_GateList, ip1, ip2);
		}
	}
	//buildConnet();

	file.close();
   return true;
}


/**********************************************************/
/*   class CirMgr private member functions    			  */
/**********************************************************/
// Called after Depth-First-Search graph traversal
void CirMgr::setRef() const{
	for(size_t i = 0, n = _GateList.size() ; i < n ; ++i)
     	if(_GateList[i] != 0) _GateList[i] -> setToGlobalRef();
	CirGate::setGlobalRef();
}
void CirMgr::buildConnet(){
	for(size_t i = 0, n = _GateList.size() ; i < n ; i++)
		if(_GateList[i] != 0)
			_GateList[i] -> buildFanIn(_GateList);
}
void CirMgr::clear_DFS() const{
	_DFSList.clear();
	_DFSmap.clear();
	_effAIGNum = 0;
}
void CirMgr::build_DFS() const{
	if(!_DFSList.empty()) return;
	assert(_DFSList.empty() and _DFSmap.empty());
	_DFSmap.insert(make_pair(0,0)); // 不論 0 是否在 _DFSList 中 皆要在 _DFSmap 中
	// Record AIGs, which are on the DFS paths.
	for(size_t i = 0, n = _PoList.size()   ; i < n ; ++i)
		_GateList[_PoList[i]] -> DFS(_DFSList, _DFSmap);
	for(size_t i = 0, n = _DFSList.size()  ; i < n ; ++i)
		if(_GateList[_DFSList[i]] -> getTypeStr() == "AIG") ++_effAIGNum;
	setRef();
}
void CirMgr::build_Fl() const{
	if(!_FlList.empty()) return;
	vector<unsigned>::iterator it;
	for(size_t i = 0, n = _GateList.size() ; i < n ; ++i)
		if(_GateList[i] != 0 and _GateList[i] -> getTypeStr() == "UNDEF")
			for(size_t j = 0, m = (_GateList[i] -> _fanOut).size() ; j < m ; ++j)
					_FlList.push_back((_GateList[i] -> _fanOut)[j].gatePtr() -> getID());
	sort(_FlList.begin(), _FlList.end());
	if(!_FlList.empty()){
		it = _FlList.begin(); ++it;
		while(it != _FlList.end())
			*it == *(it - 1) ? _FlList.erase(it) : ++it;
	}
	
}
void CirMgr::build_NU() const{
	if(!_NUList.empty()) return;
	// Record any Not-used AIG and PI whith _NUList
	for(size_t i = 0, n = _GateList.size() ; i < n ; ++i)
		if(_GateList[i] != 0)
			if((_GateList[i] -> getTypeStr() == "AIG"  or 
				 _GateList[i] -> getTypeStr() == "PI" ) and 
			    _GateList[i] -> fanOutNum()  == 0)
				_NUList.push_back(_GateList[i] -> getID());
	sort(_NUList.begin(), _NUList.end());
	
}
/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void CirMgr::printSummary() const{
	cout << endl;
	cout << "Circuit Statistics" << endl;
	cout << "==================" << endl;
	cout << "  PI " << setw(11) << right << _PiList.size()  << endl;
	cout << "  PO " << setw(11) << right << _PoList.size()  << endl;
	cout << "  AIG" << setw(11) << right << _totalGate - _PiList.size() - _PoList.size() << endl;
	cout << "------------------" << endl;
	cout << "  Total" << setw(9) << right << _totalGate << endl;
}

void CirMgr::printNetlist() const{
	build_DFS();
	size_t idx = 0;
	cout << endl;
	for(size_t i = 0, n = _DFSList.size(); i < n ; i++){
		if(_GateList[_DFSList[i]] -> getTypeStr() != "UNDEF"){
			cout << "[" << idx++ << "] "; 
			_GateList[_DFSList[i]] -> printGate();
		}
	}
	
}

void CirMgr::printPIs() const{
   	cout << "PIs of the circuit:";
   	for(size_t i = 0, n = _PiList.size() ; i < n ; i++)
		cout << " " << _PiList[i];
   	cout << endl;
}

void CirMgr::printPOs() const{
   cout << "POs of the circuit:";
   for(size_t i = 0, n = _PoList.size() ; i < n ; i++)
		cout << " " << _PoList[i];
   cout << endl;
}

void CirMgr::printFloatGates() const{
	build_Fl();
	build_NU();
	if(!_FlList.empty()){
		cout << "Gates with floating fanin(s):";
		for(size_t i = 0, n = _FlList.size() ; i < n ; i++)
			cout << " " << _FlList[i];
		cout << endl;
	}
	if(!_NUList.empty()){
		cout << "Gates defined but not used  :";
		for(size_t i = 0, n = _NUList.size() ; i < n ; i++)
			cout << " " << _NUList[i];
		cout << endl;
	}
	
}
void CirMgr::printFECPairs() const{
	for(size_t i = 0 , n = _fecGrps.size() ; i < n ; ++i){
		unsigned literal = _fecGrps[i][0];
		cout << "[" << i << "] ";
		for(size_t j = 0, m = _fecGrps[i].size() ; j < m ; ++j){
			unsigned id = _fecGrps[i][j] / 2;
			bool ivt = (_fecGrps[i][j] + literal) % 2;
			cout << (ivt ? "!" : "") << id << ((j == m - 1) ? "" : " ");
		}
		cout << endl;
	}
}

void CirMgr::writeAag(ostream& outfile) const{
	build_DFS();

	outfile << _header << " " << _miloa[0] << " " <<  _miloa[1] << " " \
			<< _miloa[2] << " " << _miloa[3] << " " << _effAIGNum << endl;
	for(size_t i = 0, n = _PiList.size()  ; i < n ; ++i)
		_GateList[_PiList[i]] -> cmdWrite(outfile);
	for(size_t i = 0, n = _PoList.size()  ; i < n ; ++i)
		_GateList[_PoList[i]] -> cmdWrite(outfile);
	for(size_t i = 0, n = _DFSList.size() ; i < n ; ++i)
		if(_GateList[_DFSList[i]] -> getTypeStr() == "AIG")
			_GateList[_DFSList[i]] -> cmdWrite(outfile);
	for(size_t i = 0, n = _PiList.size()  ; i < n ; ++i)
		if(_GateList[_PiList[i]] -> getSymbol() != "")
			outfile << "i" << i << " " << _GateList[_PiList[i]] -> getSymbol() << endl;
	for(size_t o = 0, n = _PoList.size()  ; o < n ; ++o)
		if(_GateList[_PoList[o]] -> getSymbol() != "")
			outfile << "o" << o << " " << _GateList[_PoList[o]] -> getSymbol() << endl;
	outfile << "c" << endl << "AAG output by Ting-Chun (Mike) Wang" << endl;
	
   //outfile << "c" << endl << "AAG output by Chung-Yang (Ric) Huang" << endl;
}

void CirMgr::writeGate(ostream& outfile, CirGate *g) const{
}

