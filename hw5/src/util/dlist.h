/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DLIST_H
#define DLIST_H

#include <cassert>
#include <iostream>
#include <string>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
using namespace std;

template <class T> class DList;

// DListNode is supposed to be a private class. User don't need to see it.
// Only DList and DList::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class DListNode
{
   friend class DList<T>;
   friend class DList<T>::iterator;

   DListNode(const T& d, DListNode<T>* p = 0, DListNode<T>* n = 0):
      _data(d), _prev(p), _next(n) {}

   // [NOTE] DO NOT ADD or REMOVE any data member
   T              _data;
   DListNode<T>*  _prev;
   DListNode<T>*  _next;
};


template <class T>
class DList
{
public:
   // TODO: decide the initial value for _isSorted
   DList() {
      _head = new DListNode<T>(T());
      _head -> _prev = _head -> _next = _head; // _head is a dummy node
	  _isSorted = true;
   }
   ~DList() { clear(); delete _head; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class DList;

   public:
      iterator(DListNode<T>* n= 0): _node(n) {}
      iterator(const iterator& i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return _node -> _data; }
      T& operator * () { return _node -> _data; }
      iterator& operator ++ () { (*this)++; return *this; }
      iterator operator ++ (int) { iterator tmp(*this); _node = _node -> _next; return tmp; }
      iterator& operator -- () { (*this)--; return *this; }
      iterator operator -- (int) { iterator tmp(*this); _node = _node -> _prev; return tmp; }
	  iterator operator + (int i) {iterator tmp(*this); while(i-- != 0) ++tmp; return tmp; }
	  iterator operator - (int i) {iterator tmp(*this); while(i-- != 0) --tmp; return tmp; }

      iterator& operator = (const iterator& i) { _node = i._node; return *this; }

      bool operator != (const iterator& i) const { return !(i == *this); }
      bool operator == (const iterator& i) const { return i._node == _node; }

   private:
      DListNode<T>* _node;
   };
	// TODO: implement these functions
	iterator begin() const { return iterator(_head -> _next); }
	// For class DList, end() is a dummy iterator
	iterator end() const { return iterator(_head); }
	bool empty()   const { return end() == begin(); }
	size_t size() const {
		size_t cnt = 0;
		DListNode<T>* cur = _head -> _next;
		while(cur != _head){
			cur = cur -> _next;
			cnt++;
		}
		return cnt;
	}

	void push_back(const T& x) {
		// Allocate memory dynamically, can't declare DListNode obj because of scope only in the function.
		DListNode<T>* add = new DListNode<T>(x, _head -> _prev, _head);
		_head -> _prev -> _next = add;
		_head -> _prev = add;
		_isSorted = false;
	}
	void pop_front() { erase(begin()); }	
	void pop_back()  { erase(--end()); }
	void clear()     { while(!empty()) pop_back(); }  // delete all nodes except for the dummy node

	// return false if nothing to erase
	bool erase(iterator pos) {
		if(empty() or pos == end()) return false;
		DListNode<T>* cur = pos._node;
		cur -> _prev -> _next = cur -> _next;
		cur -> _next -> _prev = cur -> _prev;
		_isSorted = false;
		delete cur;
		return true;
	}
	bool erase(const T& x) {
		if(empty()) return false;
		iterator cur = find(x);
		if(cur == end()) return false;
		else return erase(cur);
	}

/***********************[Insertion Sort]*************************************************
	void sort() const {
		if(_isSorted) return;
		DListNode<T>* lp_out = _head -> _next -> _next;
		while(lp_out != _head){
			DListNode<T>* lp_in = _head -> _next;
			while(lp_in != lp_out){
				if(lp_out -> _data < lp_in -> _data){
					DListNode<T>* tmp = lp_out -> _prev; // Record the prev dlist node.				
					// renew the nodes of the previous and the next node of outer-loop node
					lp_out -> _prev -> _next = lp_out -> _next;
					lp_out -> _next -> _prev = lp_out -> _prev;

					// renew outer-loop node
					lp_out -> _next = lp_in;
					lp_out -> _prev = lp_in -> _prev;

					// renew inner loop nodes					
					lp_in -> _prev -> _next = lp_out;
					lp_in -> _prev = lp_out;

					lp_out = tmp;
					break;
				}
				lp_in = lp_in -> _next;
			}
			lp_out = lp_out -> _next;
		}
		_isSorted = true;
	}
************************[Insertion Sort]*************************************************/
	void sort() const{
		if(!_isSorted){
			quickSort(begin(), --end());
			_isSorted = true;
		}
	}

	iterator find(const T& x) {
		if(empty()) return end();
		iterator cur = begin();
		while(cur != end()){
			if(*cur == x) return cur;
			cur++;
		}return cur;
	}	

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   DListNode<T>*  _head;     // = dummy node if list is empty
   mutable bool   _isSorted; // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] helper functions; called by public member functions
   void quickSort(iterator low, iterator high) const{
	   	iterator pi = partition(low, high);
	   	if(pi != low ) quickSort(low,  pi - 1);
	   	if(pi != high) quickSort(pi + 1, high);
   }	
   iterator partition(iterator low, iterator high) const{
	   	T pivot = *high;
		iterator j = low, pi = low - 1;
		while(j != high){
			if(*j < pivot){
				pi++;
				swap(*j, *pi);
			}j++;
		}
		swap(*(++pi), *high);
		return pi;
   }
};

#endif // DLIST_H
