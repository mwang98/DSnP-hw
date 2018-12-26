/****************************************************************************
FileName     [ myMinHeap.h ]
PackageName  [ util ]
Synopsis     [ Define MinHeap ADT ]
Author       [ Chung-Yang (Ric) Huang ]
Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_MIN_HEAP_H
#define MY_MIN_HEAP_H

#include <algorithm>
#include <vector>

template <class Data>
class MinHeap
{
	public:
	MinHeap(size_t s = 0) { if (s != 0) _data.reserve(s); }
	~MinHeap() {}

	void clear() { _data.clear(); }

	// For the following member functions,
	// We don't respond for the case vector "_data" is empty!
	const Data& operator [] (size_t i) const { return _data[i]; }
	Data& operator [] (size_t i) { return _data[i]; }

	size_t size() const { return _data.size(); }

	// TODO
	const Data& min() const { 
		if(_data.empty()) return Data();
		return (*_data.begin());
	}
	void insert(const Data& d){
		_data.push_back(d);
		reshape_up(size() - 1);
	}
	void delMin() {
		if(_data.empty()) return;
		_data[0] = _data[size() - 1];
		_data.erase(--_data.end());
		if(!_data.empty()) reshape_down(0);
	}
	void delData(size_t i) {
		size_t parent;
		i % 2 == 0 ? parent = i/2 - 1 : parent = i/2;
		
		_data[i] = _data[size() - 1];
		_data.erase(--_data.end());
		if(!_data.empty()){
			// Top node should only go down if necessary
			if(i == 0) reshape_down(i);
			// End node should do nothing
			else if(i == size());
			// Other nodes depend on their workload
			else _data[i] < _data[parent] ? reshape_up(i) : reshape_down(i);
		}
	}
	void reshape_up(int index){
		if(index == 0) return;
		int parent;
		index % 2 == 0 ? parent = index/2 - 1 : parent = index/2;
		if(_data[index] < _data[parent]){
			swap(_data[parent], _data[index]);
			reshape_up(parent);
		}
	}
	void reshape_down(int index){
		size_t left = index * 2 + 1, right = index * 2 + 2;
		// No child
		if(left >= size()) return;
		size_t min = left;
		// For Two children
		if(right < size() and  _data[right] < _data[min]) min = right;

		if(_data[min] < _data[index]){
			swap(_data[min], _data[index]);
			reshape_down(min);
		}
	}
	/*
	void print(){
		size_t cnt = 2;
		for(size_t i = 0 ; i < _data.size() ; ++i){
			if( i == cnt-1){
				cnt *=2;
				cout << endl;
			}
			cout << _data[i] << " "; 
		}
		cout << endl;
	}
	*/
	private:
	// DO NOT add or change data members
	vector<Data>   _data;
};

#endif // MY_MIN_HEAP_H
