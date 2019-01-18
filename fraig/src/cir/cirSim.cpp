/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <random>
#include <string>
#include <regex>
#include <climits>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
default_random_engine engine(0);
uniform_int_distribution<size_t> rngen(0 , ULONG_MAX);
bool myCompare4vec(const vector<unsigned>& v1, const vector<unsigned>& v2){
	assert(v1.size() > 0 and v2.size() > 0 );
	return(v1[0] < v2[0]);
}
regex pattern_sim("[01]+");
/**************************************/
/*   Static varaibles and functions   */
/**************************************/
void show_sim(size_t a){
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
/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void CirMgr::randomSim(){
	size_t random_num = (_totalGate + _PoList.size() * 2), iteration_num = 0 , cnt = 0;
	random_num / 64 == 0 ? random_num : random_num /= 64;
	build_DFS();
	initFecGrps();
	
	if(!_CEXipPattern.empty()){
		size_t ran_num = random_num * pow(0.6, _numSim);
		iteration_num  = _CEXipPattern.size() + ran_num;
		assert(ran_num <= random_num);
		for(size_t i = 0 ; i < _CEXipPattern.size() ; ++i){
			doSimulation(_CEXipPattern[i]);
			writeSimulation(_CEXipPattern[i], 64);
		}
		for(size_t i = 0 ; i < ran_num ; ++i){
			vector<size_t> pattern; pattern.reserve(_PiList.size());
			for(size_t i = 0, n = _PiList.size() ; i < n ; ++i)
				pattern.push_back(rngen(engine));
			doSimulation(pattern);
			writeSimulation(pattern, 64);
		}
		_numSim++;
	}
	else
		while(cnt++ != random_num){
			iteration_num = random_num;
			vector<size_t> pattern; pattern.reserve(_PiList.size());
			for(size_t i = 0, n = _PiList.size() ; i < n ; ++i)
				pattern.push_back(rngen(engine));
			doSimulation(pattern);
			writeSimulation(pattern, 64);
		}

	cout << iteration_num*64 << " patterns simulated." << endl;
	for(size_t i = 0, n = _fecGrps.size() ; i < n ; i++)
		sort(_fecGrps[i].begin(), _fecGrps[i].end());
	sort(_fecGrps.begin(), _fecGrps.end(), myCompare4vec);
	
	for(size_t i = 0, n = _fecGrps.size() ; i < n ; i++){
		vector<unsigned>* fecGrp = &(_fecGrps[i]);
		for(size_t j = 0, m = _fecGrps[i].size() ; j < m ; ++j)
			_GateList[_fecGrps[i][j] / 2] -> setfecPos(fecGrp);
	}
}
void CirMgr::fileSim(ifstream& patternFile){
	size_t cnt = 0;
	vector<size_t> pattern(_PiList.size(), 0);
	build_DFS();
	initFecGrps();
	
	while(patternFile.peek() != EOF){
		string input;
		patternFile >> input;
		if(input.size() != pattern.size()){
			if(input.size() != 0){
				cerr << "Pattern(" << input << ") length(" << input.size() << \
					") does not match the number of inputs("<< pattern.size() << ") in a circuit!!" << endl;
				break;
			}
			else break;
		}	
		else if(!regex_match(input, pattern_sim)){
			cerr << "Error: Pattern(" << input << ") contains a non-0/1 character('')." << endl;
			break;
		}
		for(size_t i = 0, n = pattern.size() ; i < n ; ++i){
			size_t add;
			input[i] == '0' ? add = 0 : add = (size_t)1 << (cnt%64);
			pattern[i] = pattern[i] | add;
		}
		++cnt;

		if(cnt%64 == 0){
			doSimulation(pattern);
			writeSimulation(pattern, 64);
			for(size_t i = 0 ; i < pattern.size() ; ++i) pattern[i] = 0;
		}
	}
	if(cnt%64 != 0){
		doSimulation(pattern);
		writeSimulation(pattern, cnt%64);
	}

	cout << cnt << " patterns simulated." << endl;
	for(size_t i = 0, n = _fecGrps.size() ; i < n ; i++)
		sort(_fecGrps[i].begin(), _fecGrps[i].end());
	sort(_fecGrps.begin(), _fecGrps.end(), myCompare4vec);
	
	for(size_t i = 0, n = _fecGrps.size() ; i < n ; i++){
		vector<unsigned>* fecGrp = &(_fecGrps[i]);
		for(size_t j = 0, m = _fecGrps[i].size() ; j < m ; ++j)
			_GateList[_fecGrps[i][j] / 2] -> setfecPos(fecGrp);
	}
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
// For random simulation or file simulation
void CirMgr::doSimulation(const vector<size_t>& pattern){
	size_t pos = _fecGrps.size();
	build_DFS();
	assert(_PiList.size() == pattern.size());

	for(size_t i = 0, n = _PiList.size() ; i < n ; ++i)
		_GateList[_PiList[i]] -> setValue(pattern[i]);
	for(size_t i = 0, n = _DFSList.size() ; i < n ; ++i)
		_GateList[_DFSList[i]] -> simulation();
	//for(size_t i = 0, n = _PoList.size() ; i < n ; ++i)
		//_GateList[_PoList[i]] -> simulation();
	// Traverse the graph once
	//setRef();
	
	for(size_t i = 0, n = _fecGrps.size() ; i < n ; ++i){
		unordered_map<size_t, vector<unsigned> > hash;
		unordered_map<size_t, vector<unsigned> >::iterator it;

		if(_fecGrps[i].size() == 2){
			size_t v1 = _GateList[_fecGrps[i][0]/2] -> getValue();
			size_t v2 = _GateList[_fecGrps[i][1]/2] -> getValue();
			if(v1 == v2 or v1 == ~v2)
				_fecGrps.push_back(_fecGrps[i]);
		}
		else{
			for(size_t j = 0, m = _fecGrps[i].size() ; j < m ; ++j){
				size_t value = _GateList[_fecGrps[i][j]/2] -> getValue();
				bool invert = false;
				it = hash.find(value);
				if(it == hash.end()){
					it = hash.find(~value);
					if(it != hash.end()) invert = true;
				}
				if(it == hash.end()){
					vector<unsigned> tmp = {_GateList[_fecGrps[i][j]/2] -> getID()*2};
					hash.insert(make_pair(value, tmp));
				}
				else{
					unsigned literal;
					invert ? literal = _GateList[_fecGrps[i][j]/2] -> getID()*2 + 1 : \
							 literal = _GateList[_fecGrps[i][j]/2] -> getID()*2;
					(*it).second.push_back(literal);
				}
			}
		}
		for(it = hash.begin() ; it != hash.end() ; ++it)
			if((*it).second.size() > 1)
				_fecGrps.push_back((*it).second);	
	}
	_fecGrps.erase(_fecGrps.begin(), _fecGrps.begin() + pos);
}
void CirMgr::initFecGrps(){
	// Simulation 後 _fecPos 會更新，故將他設為預設 NULL pointer
	for(size_t i = 0, n = _GateList.size() ; i < n ; i++)
		if(_GateList[i] != 0) _GateList[i] -> setfecPos(0);

	build_DFS();
	// Simulation() can be called multiple times and _fecGrps will accumulate
	if(_fecGrps.empty()){
		// Only AIGs and CONST are in _fecGrps
		vector<unsigned> buff = {0};
		for(size_t i = 0, n = _DFSList.size() ; i < n ; ++i){
			if(_GateList[_DFSList[i]] -> getTypeStr() != "AIG") continue;
			buff.push_back(_GateList[_DFSList[i]] -> getID() * 2);
		}
		_fecGrps.push_back(buff);
	}
}
void CirMgr::writeSimulation(const vector<size_t>& pattern, size_t num){
	if(_simLog != 0){
		for(size_t i = 0 ; i < num ; i++){
			size_t getter = 1; getter <<= i;
			for(size_t j = 0, m = _PiList.size() ; j < m ; ++j){
				size_t tmp = getter & pattern[j];
				(*_simLog) << (tmp == getter ? "1" : "0");
			}
			(*_simLog) << " ";
			for(size_t j = 0, m = _PoList.size() ; j < m ; ++j){
				size_t tmp = getter & _GateList[_PoList[j]] -> getValue();
				(*_simLog) << (tmp == getter ? "1" : "0");
			}	
			(*_simLog) << endl;
		}
	}
}