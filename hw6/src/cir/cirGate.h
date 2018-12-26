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
#include <algorithm>
#include "cirDef.h"

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes

// The LSB is 1 iff an invert bubble is between nodes
class GateV{
	public:
		GateV(CirGate* gate, bool invert){_gate = reinterpret_cast<size_t>(gate) + invert; }
		CirGate* gatePtr() const{ return reinterpret_cast<CirGate*>(_gate & ~size_t(1)); }
		bool is_invert()   const{ return (_gate & size_t(1)); }
	private:
		size_t _gate;
};

class CirGate
{
public:
	CirGate(unsigned id, unsigned ln = 0): _id(id), _lineNum(ln), \
	_symbol(""), _ref(0){}
	virtual ~CirGate() {}

	// Basic access methods
	virtual  string getTypeStr	  ()const = 0;
	unsigned getLineNo		      ()const{ return _lineNum; }
	unsigned getID			  	  ()const{ return _id; 		}
	string   getSymbol		  	  ()const{ return _symbol; 	}
	unsigned getRef		      	  ()const{ return _ref; 	}
	void 	 setSymbol(const string& str){ _symbol = str; }
	void 	 setToGlobalRef		  ()const{ _ref = _globalRef; }

	// Building connections between nodes
			void buildFanIn(vector<CirGate*>&, vector<unsigned>& floatList);
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
		   void 	DFS(vector<unsigned>& DFSList)const;

protected:
	unsigned 		  _id;
	unsigned 		  _lineNum;
	mutable unsigned  _ref;
	string 	 		  _symbol;
	vector<GateV> 	  _fanIn;
	vector<GateV> 	  _fanOut;
	vector<unsigned>  _ip; 			// ID of the input gates

};
class PiGate : public CirGate{
	public:
		PiGate(unsigned id, unsigned ln): CirGate(id, ln){}
		~PiGate(){}
		string getTypeStr() const{ return "PI"; }

		void printGate() 		const;
		void cmdWrite(ostream&) const;
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
		PoGate(unsigned id, unsigned ln, unsigned ip): CirGate(id, ln){ _ip.push_back(ip);}
		~PoGate(){}
		string getTypeStr() const{ return "PO"; }

		void buildFanOut(GateV*&){return;}

		void printGate() 		const;
		void cmdWrite(ostream&) const;
};
class AIGate : public CirGate{
	public:
		AIGate(unsigned id, unsigned ln, unsigned ip1, unsigned ip2): \
		CirGate(id, ln){_ip.push_back(ip1); _ip.push_back(ip2);}
		~AIGate(){}
		string getTypeStr() const{ return "AIG"; }

		void printGate() 		const;
		void cmdWrite(ostream&) const;
};


#endif // CIR_GATE_H
