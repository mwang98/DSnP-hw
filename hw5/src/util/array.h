/****************************************************************************
  FileName     [ array.h ]
  PackageName  [ util ]
  Synopsis     [ Define dynamic array package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>
#include <algorithm>
#include <cmath>

using namespace std;

// NO need to implement class ArrayNode
//
template <class T>
class Array
{
public:
    // TODO: decide the initial value for _isSorted
    Array() : _data(0), _size(0), _capacity(0) { _isSorted = false; }
    ~Array() { delete []_data; }

    // DO NOT add any more data member or function for class iterator
    class iterator
    {
        friend class Array;

    public:
        iterator(T* n= 0): _node(n) {}
        iterator(const iterator& i): _node(i._node) {}
        ~iterator() {} // Should NOT delete _node

            // TODO: implement these overloaded operators
        const T& operator * () const { return (*_node); }
        T& operator * () { return (*_node); }
        iterator& operator ++ () { (*this)++ ; return *this; }
        iterator operator ++ (int) { iterator tmp(*this); _node++; return tmp; }
        iterator& operator -- () { (*this)-- ; return *this; }
        iterator operator -- (int) { iterator tmp(*this); _node--; return tmp; }
        iterator operator + (int i) const { iterator tmp(*this) ; while(i-- != 0) ++tmp; return tmp; }
        iterator& operator += (int i) { _node += i; return *this; }

        iterator& operator = (const iterator& i) { _node = i._node; return *this; }

        bool operator != (const iterator& i) const { return !(i == *this); }
        bool operator == (const iterator& i) const { return _node == i._node; }

        private:
            T*    _node;
    };

    // TODO: implement these functions
    // Tested
    iterator begin() const { return iterator(_data); }
    iterator end()   const { return iterator(_data + _size); }
    bool     empty() const { return _size == 0; }
    size_t   size()  const { return _size; }

    T&       operator [] (size_t i) { return _data[i]; }
    const T& operator [] (size_t i) const { return _data[i]; }

    void push_back(const T& x) {
        if(_size == _capacity) expand();
        _data[_size] = x;
        _size++;
        _isSorted = false;
    }
    void pop_front() {
        if(empty()) return;
        erase(begin());
    }
    void pop_back() {
        if(empty()) return;
        erase(--end());
    }

    // replace the argument entry with the last entry
    bool erase(const T& x) {
        if(empty()) return false;
        iterator cur = find(x);
        if(cur == end()) return false;
        else return erase(cur);
    }
    bool erase(iterator pos) {
        if(empty() or pos == end()) return false;
        *pos = _data[--_size];
        _isSorted = false;
        return true;
    }

    // NO need to release memory
    void clear() {
        _size = 0;
        _isSorted = false;
    }

    // [Optional TODO] Feel free to change, but DO NOT change ::sort()
    void sort() const { 
        if(_isSorted) return;
        if (!empty()){
            ::sort(_data, _data+_size);
            _isSorted = true;
        }
    }

    iterator find(const T& x) {
        if(empty()) return end();
        size_t pos = 0;
        while(pos != _size and x != _data[pos]) pos++;
        if(pos == _size) return end();
        else return iterator(_data + pos);
    }

    // Nice to have, but not required in this homework...
    // void reserve(size_t n) { ... }
    // void resize(size_t n) { ... }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   T*            _data;
   size_t        _size;       // number of valid elements
   size_t        _capacity;   // max number of elements
   mutable bool  _isSorted;   // (optionally) to indicate the array is sorted

   void expand(){
        _capacity == 0 ? _capacity = 1 : _capacity *= 2;
        T* buff = new T[_capacity];

        for(size_t i = 0 ; i < _size ; i++) buff[i] = _data[i];
        delete [] _data; 
        _data = buff;
   }

   // [OPTIONAL TODO] Helper functions; called by public member functions
};

#endif // ARRAY_H

