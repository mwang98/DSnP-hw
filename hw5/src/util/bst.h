/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

using namespace std;

template <class T> class BSTree;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class BSTreeNode
{
    friend class BSTree<T>;
    friend class BSTree<T>::iterator;

private:
    BSTreeNode(){}
    BSTreeNode(const T& e, const short& s = 0):
    _element(e), _left(NULL), _right(NULL), _parent(NULL), _state(s)  {} // Private Constructor

    BSTreeNode<T>* _left  ;
    BSTreeNode<T>* _right ;
    BSTreeNode<T>* _parent;
    T _element;
    bool _state; // 0 - normal node ;1 - _tail;
};

template <class T>
class BSTree
{
    friend class iterator;
public:
    BSTree(){
        _tail  = new BSTreeNode<T>(T(),1);
        _root = _tail;
        _size = 0;
        _connectT = true;
    }
    ~BSTree(){ clear(); delete _tail; }
    
    class iterator {
        friend class BSTree;
    public:
        iterator(BSTreeNode<T>* n = 0):_node(n){}
        iterator(const iterator& i):_node(i._node){}
        ~iterator(){};

        const T& operator * () const { return _node -> _element; }
        T& operator * () { return _node -> _element; }
        //increment : in-order traversal
        iterator& operator ++ () { (*this)++; return *this; }
        iterator operator ++ (int) {
            iterator it(*this);
            // Case1 : Final state : _tail Node
            if(_node -> _state == 1);
            // Case2 : 移動到右支，不斷向左邊找
            else if(_node -> _right != NULL){
                BSTreeNode<T>* cur = _node -> _right;
                while(cur -> _left != NULL) cur = cur -> _left;
                _node = cur;
            }
            // Case3 : 向上回溯
            else{
                BSTreeNode<T>* cur = _node -> _parent;
                while(cur -> _element <= _node -> _element and cur -> _parent != 0) cur = cur -> _parent;
                if(cur -> _element > _node -> _element) _node = cur;
            }    
            return it;
        }
        iterator& operator -- () { (*this)--; return *this;}
        iterator operator -- (int) {
            iterator it(*this);
            // Case1 : 從 _tail 出發，不比較 element
            if(_node -> _state == 1) _node = _node -> _parent;
            // Case2 : 移動到左支，不斷向右邊找
            else if(_node -> _left != NULL){
                BSTreeNode<T>* cur = _node -> _left;
                while(cur -> _right != NULL) cur = cur -> _right;
                _node = cur;
            }
            // Case3 : 向上回溯
            else{
                BSTreeNode<T>* cur = _node -> _parent;
                if(cur != 0){
                    while(cur -> _element > _node -> _element and cur -> _parent != 0) cur = cur -> _parent;
                    if(cur -> _element <= _node -> _element) _node = cur;
                }
            }
            return it;
        }
        iterator& operator = (const iterator& i) { this -> _node = i._node; return *this; }
        bool operator != (const iterator& i) const { return !(i == *this); }
        bool operator == (const iterator& i) const { return i._node == _node; }
        
    private:
        BSTreeNode<T>* _node;
    };

    iterator begin() const{
        BSTreeNode<T>* ptr = _root;
        if(empty()) return iterator(_root);
        findMin(_root, ptr);
        return iterator(ptr);
    }
    iterator end() const{
        if(empty()) return iterator(_root);
        return iterator(_tail);
    }
    void insert(const T& x){
        disconnectTail();
        BSTreeNode<T>* add = new BSTreeNode<T>(x);
        BSTreeNode<T>* cur = _root;

        if(empty()) _root = add;
        else{
            while(true){
                if(add -> _element < cur -> _element){
                    if(cur -> _left == 0){
                        cur -> _left = add;
                        add -> _parent = cur;
                        break; 
                    } cur = cur -> _left;
                }
                else{
                    if(cur -> _right == 0){
                        cur -> _right = add;
                        add -> _parent = cur;
                        break; 
                    } cur = cur -> _right;
                }
            }
        }
        _size++;
        reconnectTail();
    }
    void pop_back(){
        if(empty()) return;
        erase(--end());
    }
    void pop_front(){
        if(empty()) return;
        erase(begin());
    }
    bool erase(const T& x){
        BSTreeNode<T>* ptr = _root;
        if(empty()) return false;
        if(search(ptr, x)) return erase(iterator(ptr));
        return false;
    }
    bool erase(iterator pos){
        if(empty() or pos == end()){ return false; }
        if(_connectT) disconnectTail();

        // Case1 : NO child
        if(pos._node -> _left == NULL and pos._node -> _right == NULL){
            if(pos._node == _root){ _root = _tail; }
            else{
                if(pos._node -> _parent -> _left == pos._node)
                    pos._node -> _parent -> _left = NULL;
                else
                    pos._node -> _parent -> _right = NULL;
            }
            _size--;
            delete pos._node;
        }
        // Case2 : Two children
        else if(pos._node -> _left != NULL and pos._node -> _right != NULL){
            // Degenerate the problem to Case 3 OR Case 1
            iterator tmp(pos._node); ++tmp;
            swap(*pos, *tmp);
            erase(tmp);
        }
        // Case3 : One child
        else{
            if(pos._node == _root){
                if(pos._node -> _left != NULL){
                    _root = pos._node -> _left;
                    pos._node -> _left -> _parent = _root;
                }
                else{
                    _root = pos._node -> _right;
                    pos._node -> _right -> _parent = _root;
                }
            }
            // pos is left child
            else if(pos._node -> _parent -> _left == pos._node){
                // child of pos is left
                if(pos._node -> _left != NULL){
                    pos._node -> _parent -> _left = pos._node -> _left;
                    pos._node -> _left -> _parent = pos._node -> _parent;
                }
                // child of pos is right
                else{
                    pos._node -> _parent -> _left = pos._node -> _right;
                    pos._node -> _right -> _parent = pos._node -> _parent;
                }
            }
            // pos is right child
            else if(pos._node -> _parent -> _right == pos._node){
                if(pos._node -> _left != NULL){
                    pos._node -> _parent -> _right = pos._node -> _left;
                    pos._node -> _left -> _parent = pos._node -> _parent;
                }
                else{
                    pos._node -> _parent -> _right = pos._node -> _right;
                    pos._node -> _right -> _parent = pos._node -> _parent;
                }
            }
            _size--;
            delete pos._node;
        }
        if(!_connectT) reconnectTail();
        return true;
    }
    size_t size () const{ return _size; }
    bool   empty() const{ return _size == 0; }
    void   sort () const{ return; }
    void   clear(){ for(size_t i = 0, cnt = size() ; i < cnt ; i++) pop_front(); }
    iterator find(const T& x) const{
        BSTreeNode<T>* cur = _root;
        if(empty()) return end();
        if(search(cur, x)) return iterator(cur);
        else return end();
    }
//-----------------------For debug-------------------------
    void print() const{
        if(empty());
        else{
           print(_root);
           cout << endl;
           pre_order(_root);
        }
    }
    void print(const BSTreeNode<T>* ptr) const{
        if(ptr -> _left != NULL) print(ptr -> _left);

        if(ptr -> _left != NULL and ptr -> _right != NULL and ptr -> _right != _tail)
            cout << "(" << setw(7) << ptr -> _left -> _element << ", " << setw(7) << ptr -> _element << ", " << setw(7) << ptr -> _right -> _element << ") " << &(ptr -> _element) << "\n";
        if(ptr -> _left == NULL and ptr -> _right != NULL and ptr -> _right != _tail)
            cout << "(NULL   , " << setw(7) << ptr -> _element << ", " << setw(7) << ptr -> _right -> _element << ") " << &(ptr -> _element) << "\n";
        if(ptr -> _left != NULL and (ptr -> _right == NULL or ptr -> _right == _tail ))
            cout << "(" << setw(7) << ptr -> _left -> _element << ", " << setw(7) << ptr -> _element << ", NULL   ) " << &(ptr -> _element) << "\n";
        if(ptr -> _left == NULL and (ptr -> _right == NULL or ptr -> _right == _tail ))
            cout << "(NULL   , " << setw(7) << ptr -> _element << ", NULL   ) " << &(ptr -> _element) << "\n";
        
        if(ptr -> _right != NULL and ptr -> _right != _tail) print(ptr -> _right);
    }
    void pre_order(const BSTreeNode<T>* ptr) const{
        if(ptr -> _left != NULL and ptr -> _right != NULL and ptr -> _right != _tail)
            cout << "(" << setw(7) << ptr -> _left -> _element << ", " << setw(7) << ptr -> _element << ", " << setw(7) << ptr -> _right -> _element << ") " << &(ptr -> _element) << "\n";
        if(ptr -> _left == NULL and ptr -> _right != NULL and ptr -> _right != _tail)
            cout << "(NULL   , " << setw(7) << ptr -> _element << ", " << setw(7) << ptr -> _right -> _element << ") " << &(ptr -> _element) << "\n";
        if(ptr -> _left != NULL and (ptr -> _right == NULL or ptr -> _right == _tail ))
            cout << "(" << setw(7) << ptr -> _left -> _element << ", " << setw(7) << ptr -> _element << ", NULL   ) " << &(ptr -> _element) << "\n";
        if(ptr -> _left == NULL and (ptr -> _right == NULL or ptr -> _right == _tail ))
            cout << "(NULL   , " << setw(7) << ptr -> _element << ", NULL   ) " << &(ptr -> _element) << "\n";

        if(ptr -> _left != NULL) pre_order(ptr -> _left);
        if(ptr -> _right != NULL and ptr -> _right != _tail) pre_order(ptr -> _right);
    }
//---------------------------------------------------------

private:
    BSTreeNode<T>*  _root;
    BSTreeNode<T>*  _tail;
    size_t          _size;
    mutable bool    _connectT;

    bool search(BSTreeNode<T>*& ptr, const T& target) const{
        if(ptr == NULL or ptr == _tail) return false;
        if(ptr -> _element == target) return true;
        else if(target > ptr -> _element) return search((ptr = ptr -> _right), target);
        else if(target < ptr -> _element) return search((ptr = ptr -> _left ), target);
        return false;
    }
    bool findMin(BSTreeNode<T>* cur, BSTreeNode<T>*& rt) const{
        rt = cur;
        if(cur -> _left == 0) return false;
        while(cur -> _left != 0) cur = cur -> _left;
        rt = cur;
        return true;
    }
    bool findMax(BSTreeNode<T>* cur, BSTreeNode<T>*& rt) const{
        rt = cur;
        if(cur -> _right == 0 or cur -> _right == _tail) return false;
        while(cur -> _right != 0 and cur -> _right != _tail) cur = cur -> _right;
        rt = cur;
        return true;
    }
    void disconnectTail() const{
        BSTreeNode<T>* cur;
        findMax(_root, cur);
        cur -> _right = 0;
        _tail -> _parent = 0;
        _connectT = false;
    }
    void reconnectTail() const{
        BSTreeNode<T>* cur;
        findMax(_root, cur);
        cur -> _right = _tail;
        _tail -> _parent = cur;
        _connectT = true;
    }
};

#endif // BST_H
