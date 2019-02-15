/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;
class GateV;
//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
	friend class CirMgr;
public:
	CirGate(unsigned id, unsigned ln = 0):_value(0), _id(id), _lineNum(ln), _ref(0), _symbol(""), _fecPos(0){}
	virtual ~CirGate() {}

	// Basic access methods
	virtual  string getTypeStr()const = 0;
	unsigned getLineNo()        const{ return _lineNum; }
	unsigned getID()            const{ return _id; }
	string   getSymbol()        const{ return _symbol; }
	unsigned getRef()           const{ return _ref; }
	size_t   getValue()			const{ return _value; }
	void 	 setSymbol(const string& str){ _symbol = str; }
	void 	 setToGlobalRef()   const{ _ref = _globalRef; }
  	virtual bool isAig()        const{ return false; }

	// Building connections between nodes
			void buildFanIn(vector<CirGate*>&);
			void buildFanIn(vector<CirGate*>&, int, int);
	virtual void buildFanOut(const GateV&);

	int fanOutNum(){ return _fanOut.size(); }

	// Printing functions
	virtual void printGate	 () const = 0;
			void reportGate  () const;
			void reportFanin (int level) const{ reportFanin (level, level); resetToGlobalRef();}
			void reportFanout(int level) const{ reportFanout(level, level); resetToGlobalRef();}
			void reportFanin (const int level, int nlv) const;
			void reportFanout(const int level, int nlv) const;
			void resetToGlobalRef() const;
	virtual	void cmdWrite(ostream&) const{ return; } //Only for Command: Write

	// For DFS traversal
	static void 	setGlobalRef(){ _globalRef++; }
	static unsigned getGlobalRef(){ return _globalRef; }
	static unsigned  _globalRef;
		   void 	DFS(vector<unsigned>&, unordered_map<unsigned, unsigned>&)const;

	// For simulation()
	virtual void simulation(){ return; }
	        void setValue(const size_t& pattern){ _value = pattern; }
					void setfecPos(vector<unsigned>* ptr){ _fecPos = ptr; }
	vector<unsigned>* getfecPos() {return _fecPos; }

	// For fraig()
		    void setSATid(const Var& val){ _SATid = val; }
			Var  getSATid(){ return _SATid; }

	size_t			  _value;
protected:
	unsigned 		  _id;
	unsigned 		  _lineNum;
	mutable unsigned  _ref;
	Var				  _SATid;
	string 	 		  _symbol;
	// 優化：_fanIn, _fanOut 改成 vector<unsigned>, _ip 可以不要記
	vector<unsigned>* _fecPos;
	vector<GateV> 	  _fanIn;
	vector<GateV> 	  _fanOut;

};
class PiGate : public CirGate{
	public:
		PiGate(unsigned id, unsigned ln): CirGate(id, ln){}
		~PiGate(){}
		string getTypeStr() const{ return "PI"; }
		void printGate() 		const;
		void cmdWrite(ostream&) const;
		void simulation(){ return; }
};
class ConstGate : public PiGate{
	public:
		ConstGate(): PiGate(0, 0){}
		~ConstGate(){}
		string getTypeStr() const{ return "CONST"; }
		void printGate() const;
};
class FloGate : public PiGate{
	public:
		FloGate(unsigned id): PiGate(id, 0){}
		~FloGate(){}
		string getTypeStr() const{ return "UNDEF"; }
		void printGate() const;
};
class PoGate : public CirGate{
	public:
		//PoGate(unsigned id, unsigned ln, unsigned ip): CirGate(id, ln){ _ip.push_back(ip);}
		PoGate(unsigned id, unsigned ln): CirGate(id, ln){}
		~PoGate(){}
		string getTypeStr() const{ return "PO"; }
		void buildFanOut(GateV*&){return;}
		void printGate() 		const;
		void cmdWrite(ostream&) const;
		void simulation();
};
class AIGate : public CirGate{
	public:
		/*AIGate(unsigned id, unsigned ln, unsigned ip1, unsigned ip2): \
		CirGate(id, ln){_ip.push_back(ip1); _ip.push_back(ip2);}
		*/
		AIGate(unsigned id, unsigned ln): CirGate(id, ln){}
		~AIGate(){}
		string getTypeStr() const{ return "AIG"; }
    	bool isAig() const{ return true; }
		void printGate() 		const;
		void cmdWrite(ostream&) const;
		void simulation();
};

class GateV{
	public:
		GateV(CirGate* gate, bool invert){_gate = reinterpret_cast<size_t>(gate) + invert; }
		CirGate* gatePtr() const{ return reinterpret_cast<CirGate*>(_gate & ~size_t(1)); }
		unsigned getID()   const{ return (gatePtr() -> getID()); }
		size_t   getSim()  const{
			if(is_invert()) return ~(gatePtr() -> getValue());
			else		    return   gatePtr() -> getValue();
		}
		bool is_invert()   const{ return (_gate & size_t(1)); }
	private:
		size_t _gate;
};

#endif // CIR_GATE_H
