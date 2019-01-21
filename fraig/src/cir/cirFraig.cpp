/****************************************************************************
	FileName     [ cirFraig.cpp ]
	PackageName  [ cir ]
	Synopsis     [ Define cir FRAIG functions ]
	Author       [ Chung-Yang (Ric) Huang ]
	Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>

#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <iomanip>

using namespace std;

/*******************************/
/*   Global variable and enum  */
/*******************************/
// 使 _fecGrp 中的元素按照 _DFSList 排序
bool myCompare4fec(const unsigned& v1, const unsigned& v2){
	unsigned pos1 = (*cirMgr -> _DFSmap.find(v1/2)).second;
	unsigned pos2 = (*cirMgr -> _DFSmap.find(v2/2)).second;
	return (pos1 < pos2);
}
/**************************************/
/*   Static varaibles and functions   */
/**************************************/
void show_bin(size_t a){
	size_t i = 0;
	cout << setw(20) << a << " : ";
	while(i < 64){
		if(i % 8 == 0 and i != 0) cout << '_' ;
		cout << a%2;
		a /= 2;
		++i;
	}
	cout << endl;
}
/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void CirMgr::strash(){
	// For loop with _DFSList version
	build_DFS();
	for(size_t i = 0, n = _DFSList.size() ; i < n ; ++i){
		CirGate* gate = _GateList[_DFSList[i]];
		assert(gate != 0);
		if(gate != 0 and gate -> getTypeStr() == "AIG"){
			unordered_map<string, CirGate*> :: iterator it;
			string key, ip1, ip2;
			ip1 = to_string((gate -> _fanIn[0].getID())*2 + (gate -> _fanIn[0]).is_invert());
			ip2 = to_string((gate -> _fanIn[1].getID())*2 + (gate -> _fanIn[1]).is_invert());
			ip1 < ip2 ? key = (ip1 + "_" + ip2) : key = (ip2 + "_" + ip1);
			it = _hash.find(key);
			if(it == _hash.end())
				_hash.insert(make_pair(key, gate));
			else{
				if((*it).second == gate) return;
				mergeGates((*it).second, gate, false);
				cout << "Strashing: " << (*it).second -> getID() << " merging " << gate -> getID() << "..." << endl;
				_GateList[gate -> getID()] = 0;
				delete gate;
				--_totalGate;
			}
		}
	}
	clear_DFS();
	_NUList.clear();
	_FlList.clear();
	_hash.clear();
}
void CirMgr::fraig(){
	int cnt = 0;
	initSolver();
	_CEXipPattern.clear();
	build_DFS();

	// Sort FEC Group according to its DFS position
	for(size_t i = 0, n = _fecGrps.size() ; i < n ; ++i)
		sort(_fecGrps[i].begin(), _fecGrps[i].end(), myCompare4fec);
	// Reconstruct fecGrp of gate
	for(size_t i = 0, n = _fecGrps.size() ; i < n ; i++){
		vector<unsigned>* fecGrp = &(_fecGrps[i]);
		for(size_t j = 0, m = _fecGrps[i].size() ; j < m ; ++j)
			_GateList[_fecGrps[i][j] / 2] -> setfecPos(fecGrp);
	}
	// Calling PO fanIn (DFS order)
	for(size_t i = 0, n = _PoList.size() ; i < n ; ++i){
		CirGate* ptr = _GateList[_PoList[i]];
		fraig(ptr -> _fanIn[0].gatePtr(), cnt);
	}
	// 如果無法湊滿 64 個，則 Collect 不足 64 的 Counter-Examples
	if(cnt % 64 != 0){
		vector<size_t> tmp;
		for(size_t i = 0, n = _PiList.size() ; i < n ; ++i)
			tmp.push_back(_GateList[_PiList[i]] -> getValue());
		_CEXipPattern.push_back(tmp);
	}

	setRef();
	clear_DFS();
	_FlList.clear();
	_NUList.clear();
	_fecGrps.clear();

	// Reset _fecPos (pointer to FEC group) of each gate
	for(size_t i = 0, n = _GateList.size() ; i < n ; i++)
		if(_GateList[i] != 0) _GateList[i] -> setfecPos(0);
	
	strash();
}
void CirMgr::printFEC() const{

}
/********************************************/
/*   Private member functions about fraig   */
/********************************************/
//-----------------strash---------------------
// Recursive Version
void CirMgr::strash(CirGate* gate){
	if(gate -> getRef() == CirGate :: _globalRef) return;
	if(gate -> getTypeStr() == "AIG"){
		for(size_t i = 0, n = (gate -> _fanIn).size() ; i < n ; ++i)
		strash((gate -> _fanIn[i]).gatePtr());

		unordered_map<string, CirGate*> :: iterator it;
		string key, ip1, ip2;
		ip1 = to_string((gate -> _fanIn[0].getID())*2 + (gate -> _fanIn[0]).is_invert());
		ip2 = to_string((gate -> _fanIn[1].getID())*2 + (gate -> _fanIn[1]).is_invert());
		ip1 < ip2 ? key = (ip1 + "_" + ip2) : key = (ip2 + "_" + ip1);
		it = _hash.find(key);
		if(it == _hash.end())
			_hash.insert(make_pair(key, gate));
		else{
			if((*it).second == gate) return;
			mergeGates((*it).second, gate, false);
			cout << "Strashing: " << (*it).second -> getID() << " merging " << gate -> getID() << "..." << endl;
			_GateList[gate -> getID()] = 0;
			delete gate;
			--_totalGate;
		}
	}
	gate -> setToGlobalRef();
}
// Merge the second gate to the first.
void CirMgr::mergeGates(CirGate* merger, CirGate* merged, bool ivt){
	// bool ivt 為兩個 gates 之間是否為 invert equivalence。strash() 皆為 false；fraig() 則不定。
	// merged gate 的 _fanOut 要全部 copy 到 merger；_fanOut gates 要把 merged gate 用 merger 覆蓋。 
	for(size_t i = 0, n = (merged -> _fanOut).size() ; i < n ; ++i){
		bool ivt_origin = (merged -> _fanOut)[i].is_invert();
		(merger -> _fanOut).push_back(GateV((merged -> _fanOut)[i].gatePtr(), ivt xor ivt_origin));
		for(size_t j = 0, m = (((merged -> _fanOut)[i].gatePtr() -> _fanIn).size()) ; j < m ; ++j){
			ivt_origin = ((merged -> _fanOut)[i].gatePtr() -> _fanIn)[j].is_invert();
			if(((merged -> _fanOut)[i].gatePtr() -> _fanIn)[j].gatePtr() == merged){
				((merged -> _fanOut)[i].gatePtr() -> _fanIn)[j] = GateV(merger, ivt xor ivt_origin);
				break;
			}	
		}
	}
	// merged gate 的 _fanIn gates 要把 merged gate 從其 _fanOut 移除
	for(size_t i = 0, n = (merged -> _fanIn).size() ; i < n ; ++i){
		CirGate* prev = (merged -> _fanIn)[i].gatePtr();
		vector<GateV>::iterator it = (prev -> _fanOut).begin();
		for(size_t j = 0, m = prev -> _fanOut.size() ; j < m ; ++j, ++it){
			if((prev -> _fanOut)[j].gatePtr() == merged){
				prev -> _fanOut.erase(it);
				break;
			}
		}		
	}
}
//-------------------Fraig---------------------
void CirMgr::fraig(CirGate* gate, int& cnt){
	if(gate -> getRef() == CirGate::_globalRef) return;
	if(gate -> getTypeStr() == "PI" or gate -> getTypeStr() == "UNDEF") return;

	if(cnt % 64 == 1) _switch = true;
	if(cnt % 64 == 0 and cnt != 0 and _switch == true){
		// 每次收集 64 個 Counter-Example pattern，旋即放入_CEXipPattern
		vector<size_t> tmp;
		for(size_t i = 0, n = _PiList.size() ; i < n ; ++i)
			tmp.push_back(_GateList[_PiList[i]] -> getValue());
		_CEXipPattern.push_back(tmp);
		_switch = false;

		// 每次收集 64 個，更新 _fecGrps：若 Simulation Value 不同 or Complement，則從 _fecGrps 移除
		// 確保每個 counter examples 的資訊完全利用
		for(size_t i = 0, n = _fecGrps.size() ; i < n ; ++i){
			unsigned first = _fecGrps[i][0] / 2;
			vector<unsigned>::iterator it = _fecGrps[i].begin();
			while(it != _fecGrps[i].end()){
				unsigned cmp = (*it) / 2;
				// fraig 之後的 gate ID 不會從 _fecGrps 移除
				if(_GateList[cmp] == 0){
					++it;
					continue;
				}
				if(_GateList[first] -> getValue() != _GateList[cmp] -> getValue() and 
				  ~_GateList[first] -> getValue() != _GateList[cmp] -> getValue()){
					_fecGrps[i].erase(it);
					_GateList[cmp] -> setfecPos(0);
				}
				else ++it;
			}
		}
	}
	// Recursively calling fraig() (DFS order).
	for(size_t i = 0, n = gate -> _fanIn.size() ; i < n ; ++i)
		fraig(gate -> _fanIn[i].gatePtr(), cnt);

	// After its fanIn is merged, determine whether the gate have to be merged.
	vector<unsigned>* fecPos = gate -> getfecPos();
	if(fecPos != 0){
		unsigned pos = 0;
		for(size_t i = 0, n = (*fecPos).size() ; i < n ; ++i)
			if((*fecPos)[i] / 2 == gate -> getID()){
				pos = i;
				break;
			}
		if(pos != 0){
			unsigned g1 = (*fecPos)[0] / 2;
			unsigned g2 = (*fecPos)[pos] / 2;
			bool	ivt = ((*fecPos)[0]+ (*fecPos)[pos]) % 2;
			if(_GateList[g1] -> getValue() == _GateList[g2] -> getValue() or 
			 ~(_GateList[g1] -> getValue()) == _GateList[g2] -> getValue())
				proveFEC(g1, g2, cnt, ivt);
		}
	}
	gate -> setToGlobalRef();
}
void CirMgr::initSolver(){
	_solver.initialize();
	for(size_t i = 0, n = _GateList.size() ; i < n ; ++i)
		if(_GateList[i] != 0) _GateList[i] -> setSATid(_solver.newVar());

	// CONST0 需要獨立出來建立 CNF，因為可能不在 DFS 中，但卻一定要用到。
	Var tmp = _solver.newVar();
	_solver.addAigCNF(_GateList[0] -> getSATid(), tmp, false, tmp, true);
	for(size_t i = 0, n = _PoList.size() ; i < n ; ++i)
		buildFanInCNF(_GateList[_PoList[i]] -> _fanIn[0].getID());
	
	setRef();
}
void CirMgr::buildFanInCNF(const unsigned& id){
	CirGate* gate = _GateList[id];
	// CONST0, PI, UNDEF 不需要建立 CNF
	if(id == 0) return;
	if(gate -> getTypeStr() == "PI" or gate -> getTypeStr() == "UNDEF" ) return;
	if(gate -> getRef() == CirGate::_globalRef) return;
	// Connection was built after its input gates have built connections
	for(size_t i = 0, n = gate -> _fanIn.size() ; i < n ; ++i)
		buildFanInCNF(gate -> _fanIn[i].getID());
	Var vf = gate -> getSATid();
	Var va = gate -> _fanIn[0].gatePtr() -> getSATid();
	Var vb = gate -> _fanIn[1].gatePtr() -> getSATid();
	_solver.addAigCNF(vf, va, gate -> _fanIn[0].is_invert() ,vb , gate -> _fanIn[1].is_invert());
	gate -> setToGlobalRef();
}
void CirMgr::proveFEC(const unsigned& g1, const unsigned& g2, int& cnt, bool ivt){
	Var newV = _solver.newVar();
	Var va = _GateList[g1] -> getSATid();
	Var vb = _GateList[g2] -> getSATid();
	_solver.addXorCNF(newV, va, false, vb, ivt);
	_solver.assumeRelease();
	_solver.assumeProperty(newV, true);
	if(_solver.assumpSolve()){
		// 一收集到 counter example 就更改所有 gates 的 simulation values，可使同個 fecGrp 中產生不同 sim values
		// SATsolver 可以少證明同個 fecGrp 內 gate 為等價的次數
		for(size_t i = 0, n = _DFSList.size() ; i < n ; ++i){
			CirGate* gate = _GateList[_DFSList[i]];
			if(gate == 0) continue;	
			size_t add;
			int    val = 0;
			Var vf = gate -> getSATid();
			val = _solver.getValue(vf);
			val == -1 ? add = 0 : add = val;
			gate -> _value <<= 1;
			gate -> _value |= add;
		}
		++cnt;
 	}
	else{
		cout << "Fraig: " << g1 << " merging " << (ivt ? "!" : "") << g2 <<"..." << endl;
		mergeGates(_GateList[g1], _GateList[g2], ivt);
		delete _GateList[g2];
		_GateList[g2] = 0;
		--_totalGate;
	}
}
