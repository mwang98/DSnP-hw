/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <algorithm>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
enum Opt_case{
	NA, identical, invert, const0, const1
};
/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void CirMgr::sweep(){
	build_NU();
	vector<unsigned> tmp = _NUList;
	for(size_t i = 0, n = tmp.size() ; i < n ; ++i)
			sweep(_GateList[tmp[i]]);
	_NUList.clear();
	_FlList.clear();
}
// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void CirMgr::optimize(){
	// table 紀錄 NA case 的 gates 是否已將 inputs 更新完成。（遞迴剪枝）
	vector<bool> table((_miloa[0] + 1 + _miloa[3]), false);
	for(size_t i = 0, n = _PoList.size() ; i < n ; ++i)
		optimize(_GateList[_PoList[i]], table);

	// Reconstruct all list
	clear_DFS();
	_NUList.clear();
	_FlList.clear();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
//-------------------Sweep---------------------
void CirMgr::sweep(CirGate* gate){
	if(gate == 0) return;
	string type = gate -> getTypeStr();
	size_t size = gate -> _fanOut.size();
	unsigned id = gate -> _id;
	vector<unsigned>::iterator it;
	if(size != 0 or type == "PO" or type == "CONST")return;
	if(type == "PI") return;
	else if(type == "UNDEF"){
		// Update _GateList and release memory
		_GateList[id] = 0;
		delete gate;
		cout << "Sweeping: UNDEF(" << id << ") removed..." << endl;
	}
	else if(type == "AIG"){
		vector<unsigned> tmp_fanIn;
		for(size_t i = 0, n = gate -> _fanIn.size() ; i < n ; ++i)
			tmp_fanIn.push_back((gate -> _fanIn[i]).gatePtr() -> getID());

		// 1. Remove the gate from the _fanOut Lists of all gates listed on its _fanIn (disconnect the relation)
		for(size_t i = 0, n = tmp_fanIn.size() ; i < n ; ++i){
			vector<GateV>::iterator itv = (_GateList[tmp_fanIn[i]] -> _fanOut).begin();

			while(itv != _GateList[tmp_fanIn[i]] -> _fanOut.end())
				(*itv).gatePtr() == gate ? _GateList[tmp_fanIn[i]] -> _fanOut.erase(itv) : ++itv;
		}
		
		// 2.Update _GateList and release memory
		_GateList[id] = 0;
		delete gate;
		--_totalGate;
		cout << "Sweeping: AIG(" << id << ") removed..." << endl;

		// 3.Recursively check whether its _fanIn gates become unused
		for(size_t i = 0, n = tmp_fanIn.size() ; i < n ; ++i)
			sweep(_GateList[tmp_fanIn[i]]);
	}
}
//-------------------Optimization---------------------
void CirMgr::optimize(CirGate* gate, vector<bool>& table){
	if(gate == 0) return;
	string type = gate -> getTypeStr();
	unsigned id = gate -> getID();
	Opt_case optCase = NA;
	vector<unsigned>::iterator it;
	vector<GateV>::	  iterator itv;

	if((type != "AIG" and type != "PO") or table[id] == true) return;
	if(type == "PO"){ optimize((gate -> _fanIn)[0].gatePtr(), table); return; }

	// [Identical] and [Invert] only need to optimize one input gate
	if((gate -> _fanIn)[0].gatePtr() == (gate -> _fanIn)[1].gatePtr())
		optimize((gate -> _fanIn)[0].gatePtr(), table);
	else
		for(size_t i = 0 ; i < (gate -> _fanIn).size() ; ++i)
			optimize((gate -> _fanIn)[i].gatePtr(), table);

	CirGate* ptr = (gate -> _fanIn)[0].gatePtr();
	bool     ivt = (gate -> _fanIn)[0].is_invert();

	// Determine cases
	for(size_t i = 0, n = (gate -> _fanIn).size() ; i < n ; ++i)
		if((gate -> _fanIn[i]).gatePtr() == _GateList[0]){
			(gate -> _fanIn[i]).is_invert() ? optCase = const1 : optCase = const0;
			ptr = gate -> _fanIn[i xor 1].gatePtr();
			ivt = gate -> _fanIn[i xor 1].is_invert();
		}	
	if((gate -> _fanIn[0]).gatePtr() == (gate -> _fanIn[1]).gatePtr())
		(gate -> _fanIn[0]).is_invert() xor (gate -> _fanIn[1]).is_invert() ?
		optCase = invert : optCase = identical;
	
	if(optCase == NA){ table[id] = true; return; }
	else if(optCase == identical){
		for(size_t i = 0 , n = (gate -> _fanOut).size(); i < n ; ++i){
			// 1. 更新 output gates 的 _fanIn List。將 (this)gate 在 _fanIn 的位置蓋掉。
			GateV add(ptr, (gate -> _fanOut)[i].is_invert() xor ivt);
			for(size_t j = 0, n = (gate -> _fanOut[i]).gatePtr() -> _fanIn.size() ; j < n ; ++j){
				if(((gate -> _fanOut[i]).gatePtr() -> _fanIn)[j].gatePtr() == gate){
					((gate -> _fanOut[i]).gatePtr() -> _fanIn)[j] = add;
					break;
				}
			}
			// 2. 更新 input gate 的 _fanOut List。將所有 (this)gate 的 _fanOut 新增在 input gate 的 _fanOut。
			add = GateV((gate -> _fanOut)[i].gatePtr(), (gate -> _fanOut)[i].is_invert() xor ivt);
			(ptr -> _fanOut).push_back(add);
		}
		cout << "Simplifying: " << ptr -> getID() << " merging " << (ivt ? "!" : "") << id << "..." << endl;
	}
	else if(optCase == invert or optCase == const0){
		for(size_t i = 0, n = (gate -> _fanOut).size(); i < n ; ++i){
			GateV add(_GateList[0], (gate -> _fanOut)[i].is_invert());
			for(size_t j = 0, n = (gate -> _fanOut[i]).gatePtr() -> _fanIn.size() ; j < n ; ++j){
				if(((gate -> _fanOut[i]).gatePtr() -> _fanIn)[j].gatePtr() == gate){
					((gate -> _fanOut[i]).gatePtr() -> _fanIn)[j] = add;
					break;
				}
			}
			// 2. 更新 CONST0 的 _fanOut List。將所有 (this)gate 的 _fanOut 新增在 input gate 的 _fanOut。
			(_GateList[0] -> _fanOut).push_back((gate -> _fanOut)[i]);
		}
		cout << "Simplifying: 0 merging " << id << "..." << endl;
	}
	else if(optCase == const1){
		for(size_t i = 0, n = (gate -> _fanOut).size(); i < n ; ++i){
			GateV add(ptr, (gate -> _fanOut)[i].is_invert() xor ivt);
			for(size_t j = 0, n = (gate -> _fanOut[i]).gatePtr() -> _fanIn.size() ; j < n ; ++j){
				if(((gate -> _fanOut[i]).gatePtr() -> _fanIn)[j].gatePtr() == gate){
					((gate -> _fanOut[i]).gatePtr() -> _fanIn)[j] = add;
					break;
				}
			}
			add = GateV((gate -> _fanOut)[i].gatePtr(), (gate -> _fanOut)[i].is_invert() xor ivt);
			(ptr -> _fanOut).push_back(add);			
		}
		cout << "Simplifying: " << ptr -> getID() << " merging " << (ivt ? "!" : "") << id << "..." << endl;
	}
	// 3. The other input gate 需要把 (this)gate 從 _fanOut List 去除
	itv = (ptr -> _fanOut).begin();
	while(itv != ptr -> _fanOut.end())
		(*itv).gatePtr() == gate ? ptr -> _fanOut.erase(itv) : ++itv;

	// 4. CONST0 gate 需要把 (this)gate 從 _fanOut List 去除
	itv = (_GateList[0] -> _fanOut).begin();
	while(itv != _GateList[0] -> _fanOut.end())
		(*itv).gatePtr() == gate ? _GateList[0] -> _fanOut.erase(itv) : ++itv;

	// 5. Update the dropped gate if necessary
	switch(optCase){
		case invert: update_gate(ptr); break;
		case const0: update_gate(ptr); break;
		case const1: update_gate(_GateList[0]); break;
		default: 	 break;
	}

	// 6. Remove (this)gate from _GateList
	_GateList[id] = 0;
	delete gate;
	--_totalGate;
}
void CirMgr::update_gate(CirGate* gate){
	if(gate == 0) return;
	string type = gate -> getTypeStr();
	unsigned id = gate -> _id;
	if(gate -> _fanOut.size() != 0) return;
	if(type == "UNDEF"){
		_GateList[id] = 0;
		delete gate;
	}
}

