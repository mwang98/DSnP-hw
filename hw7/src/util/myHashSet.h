/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>
#include <iostream>
#include <iomanip>

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//
template <class Data>
class HashSet
{
public:
	HashSet(size_t b = 0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
	~HashSet() { reset(); }

	// TODO: implement the HashSet<Data>::iterator
	// o An iterator should be able to go through all the valid Data
	//   in the Hash
	// o Functions to be implemented:
	//   - constructor(s), destructor
	//   - operator '*': return the HashNode
	//   - ++/--iterator, iterator++/--
	//   - operators '=', '==', !="
	//
	class iterator
	{
		friend class HashSet<Data>;
	public:
		iterator(vector<Data>* ptr, typename vector<Data>::iterator it, size_t row, size_t num): \
		_buckets(ptr), _ptr(it), _row(row), _numBuckets(num){}
		
		const Data& operator * () const { return *_ptr; }
		/**************************** 遇到的 Bug *****************************************
		 iterator:: operator == 不能只比較 vector iterator 的值（不能只比較指向的 object 是否相同）
		 						同時需要考慮 iterator 是否在同個 _row
		 end() 為最後一個 bucket (eg. _bucket[size()-1])的 end()，
		 但因記憶體分配可能使 bucket 相連在一起，
		 進而使最後一個 bucket 的 end() 可能會是其他某個 _bucket 的 begin，
		 造成提前終止迴圈的問題。
		 *******************************************************************************/
		iterator& operator ++ () {
			// the responsibility of maintaining the iterator within the range lies totally with the caller.
			++_ptr;
			if(_ptr == _buckets[_row].end()){
				++_row;
				for(; _row < _numBuckets ; ++_row)
					if(!_buckets[_row].empty()){
						_ptr = _buckets[_row].begin();
						break;
					}
				if(_row == _numBuckets){
					_ptr = _buckets[_numBuckets-1].end();
					--_row;
				}
			}
			return *this;
		}
		iterator operator ++ (int){ iterator tmp = *this; ++*this; return tmp; }
		iterator& operator -- (){
			// If vector is empty, then begin() = end().
			if(_ptr == _buckets[_row].begin()){
				--_row;
				for(; _row >= 0 ; --_row)
					if(!_buckets[_row].empty()){
						_ptr = _buckets[_row].end();
						break;
					}	
			}
			--_ptr;
			return *this;
		}
		iterator operator -- (int){ iterator tmp = *this; --*this; return tmp; }
		bool operator == (const iterator& i) const { return (_ptr == i._ptr and _row == i._row); }
		bool operator != (const iterator& i) const { return !(*this == i); }
		iterator& operator = (const iterator& i){
			_buckets = i._buckets;
			_ptr = i._ptr;
			_row = i._row;
			_numBuckets = i._numBuckets;
			return *this;
		}
		size_t getRow() { return _row;}
		/*
		void printIt() {
			cout << "===Iterator Info===" << endl;
			cout << "[_ptr]:" << *_ptr << endl;
			cout << "[_row]:" << _row << endl;
			cout << "===================" << endl;
		}
		*/
	private:
		vector<Data>*           		_buckets;
		typename vector<Data>::iterator _ptr;
		size_t 					 		_row;
		size_t 			 				_numBuckets;
	};

	void init(size_t b) { _numBuckets = b; _buckets = new vector<Data>[b]; }
	void reset() {
		_numBuckets = 0;
		if (_buckets) { delete [] _buckets; _buckets = 0; }
	}
	void clear() { for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear(); }
	size_t numBuckets() const { return _numBuckets; }

	vector<Data>& 		operator [] (size_t i) { return _buckets[i]; }
	const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

	// TODO: implement these functions
	//
	// Point to the first valid data
	iterator begin() const {
		if(size() == 0) return end();
		typename vector<Data>::iterator it;
		for(size_t i = 0, row = 0 ; i < _numBuckets ; ++i, ++row){
			if(!_buckets[i].empty()){
				it = _buckets[i].begin();
				return iterator(_buckets, it, row, _numBuckets);
			}
		}
	}
	// Pass the end
	iterator end() const {
		return iterator(_buckets, _buckets[_numBuckets-1].end(), _numBuckets-1, _numBuckets);
	}
	// return true if no valid data
	bool empty() const { return (begin() == end()); }
	// number of valid data
	size_t size() const { 
		size_t s = 0;
		for(size_t i = 0 ; i < _numBuckets ; ++i)
			for(size_t j = 0, n = _buckets[i].size() ; j < n ; ++j)
				++s;
		return s; 
	}

	// check if d is in the hash...
	// if yes, return true;
	// else return false;
	bool check(const Data& d) const {
		size_t pos = bucketNum(d);
		for(size_t i = 0, n = _buckets[pos].size() ; i < n ; ++i)
			if(d == _buckets[pos][i]) return true;
		return false;
	}

	// query if d is in the hash...
	// if yes, replace d with the data in the hash and return true;
	// else return false;
	bool query(Data& d) const {
		size_t pos = bucketNum(d);
		for(size_t i = 0, n = _buckets[pos].size() ; i < n ; ++i)
			if(d == _buckets[pos][i]){
				d = _buckets[pos][i];
				return true;
			}
		return false;
	}

	// update the entry in hash that is equal to d (i.e. == return true)
	// if found, update that entry with d and return true;
	// else insert d into hash as a new entry and return false;
	bool update(const Data& d) {
		size_t pos = bucketNum(d);
		for(size_t i = 0, n = _buckets[pos].size() ; i < n ; ++i)
			if(d == _buckets[pos][i]){
				_buckets[pos][i] = d;
				return true;
			}
		_buckets[pos].push_back(d);
		return false;
	}

	// return true if inserted successfully (i.e. d is not in the hash)
	// return false is d is already in the hash ==> will not insert
	bool insert(const Data& d) {
		size_t pos = bucketNum(d);
		for(size_t i = 0, n = _buckets[pos].size() ; i < n ; ++i)
			if(d == _buckets[pos][i]) return false;
		_buckets[pos].push_back(d);
		return true;
	}

	// return true if removed successfully (i.e. d is in the hash)
	// return fasle otherwise (i.e. nothing is removed)
	bool remove(const Data& d) {
		size_t pos = bucketNum(d);
		typename vector<Data>:: iterator it  ( _buckets[pos].begin());
		typename vector<Data>:: iterator last( --_buckets[pos].end());
		while(it != _buckets[pos].end()){
			if(d == *it){
				*it = *last;
				_buckets[pos].erase(last);
				return true;
			}++it;
		}
		return false;
	}
	// For debug
	void print() const{
		typename vector<Data>::iterator it;
		for(size_t i = 0 ; i < _numBuckets ; ++i){
			it = _buckets[i].begin();
			cout << "NO: " << setw(4) << i << " | ";
			for(; it != _buckets[i].end() ; ++it)
				cout << *it << " ";
			cout << endl;
		}
		
		cout << "[In order]==============" << endl;
		iterator itr = begin();
		int cnt = 0;
		for(; itr != end() ; ++itr){
			cout << *itr << endl;
		}
		cout << endl <<"[Reverse]==============" << endl;
		itr = end(); --itr;
		do{
			cout << *itr << endl;
			 cnt++;
		}while(itr-- != begin());	
	}

private:
	// Do not add any extra data member
	size_t            _numBuckets;
	vector<Data>*     _buckets;

	size_t bucketNum(const Data& d) const {
		return (d() % _numBuckets); }
};

#endif // MY_HASH_SET_H
