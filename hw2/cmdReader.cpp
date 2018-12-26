/****************************************************************************
  FileName     [ cmdReader.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command line reader member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <cstring>
#include "cmdParser.h"
#include <iostream>
#include <cstdlib>

using namespace std;

//----------------------------------------------------------------------
//    Extrenal funcitons
//----------------------------------------------------------------------
void mybeep();
char mygetc(istream&);
ParseChar getChar(istream&);


//----------------------------------------------------------------------
//    Member Function for  class Parser
//----------------------------------------------------------------------
void CmdParser::readCmd() {
    //instructions in dofile 
   if (_dofile.is_open()) {
      readCmdInt(_dofile);
      _dofile.close();
   }
   else // instructions given through the keyboard
      readCmdInt(cin);
}

void CmdParser::readCmdInt(istream& istr) {
   resetBufAndPrintPrompt();

   while (1) {
      ParseChar pch = getChar(istr);
      if (pch == INPUT_END_KEY) break;
      switch (pch) {
         case LINE_BEGIN_KEY :
         case HOME_KEY       : moveBufPtr(_readBuf); break;
         case LINE_END_KEY   :
         case END_KEY        : moveBufPtr(_readBufEnd); break;
         case BACK_SPACE_KEY : backSpaceChar(); break;
         case DELETE_KEY     : deleteChar(); break;
         case NEWLINE_KEY    : addHistory();
                               cout << char(NEWLINE_KEY);
                               resetBufAndPrintPrompt(); break;
         case ARROW_UP_KEY   : moveToHistory(_historyIdx - 1); break;
         case ARROW_DOWN_KEY : moveToHistory(_historyIdx + 1); break;
         case ARROW_RIGHT_KEY: moveBufPtr(_readBufPtr + 1); break;
         case ARROW_LEFT_KEY : moveBufPtr(_readBufPtr - 1); break;
         case PG_UP_KEY      : moveToHistory(_historyIdx - PG_OFFSET); break;
         case PG_DOWN_KEY    : moveToHistory(_historyIdx + PG_OFFSET); break;
         case TAB_KEY        : {
            int insert = 0, size = 0;
            char* cur = _readBuf;
            while((cur++) != _readBufPtr) size++;
            insert = TAB_POSITION - size % (int)TAB_POSITION;
            insertChar(' ',insert);
         }break;
         case INSERT_KEY     : // not yet supported; fall through to UNDEFINE
         case UNDEFINED_KEY:   mybeep(); break;
         default:  // printable character
            insertChar(char(pch)); break;
      }
      #ifdef TA_KB_SETTING
      taTestOnly();
      #endif 
   }
}


// This function moves _readBufPtr to the "ptr" pointer
// It is used by left/right arrowkeys, home/end, etc.
//
// Suggested steps:
// 1. Make sure ptr is within [_readBuf, _readBufEnd].
//    If  not, make a beep sound and return false. (DON'T MOVE)
// 2. Move the cursor to the left or right, depending on ptr
// 3. Update _readBufPtr accordingly. The content of the _readBuf[] will
//    not be changed
//
// [Note] This function can also be called by other member functions below
//        to move the _readBufPtr to proper position.
bool CmdParser::moveBufPtr(char* const ptr) {
     // Boundary check (continuous memory allocation)
    if (ptr >= _readBuf and ptr <= _readBufEnd) {
        if (ptr == _readBuf) cout << "\rcmd> ";
        else if (ptr == _readBufEnd) cout << "\rcmd> " << _readBuf;
        else if (ptr < _readBufPtr) cout << '\b';
        else if (ptr > _readBufPtr) {
            char* cur = _readBuf;
            cout << "\rcmd> ";
            while (cur != ptr) cout << *(cur++);
        }
        _readBufPtr = ptr;
    }
    else {
        mybeep();
        return false;
    }
    return true;
}

// [Notes]
// 1. Delete the char at _readBufPtr
// 2. mybeep() and return false if  at _readBufEnd
// 3. Move the remaining string left for  one character
// 4. The cursor should stay at the same position
// 5. Remember to update _readBufEnd accordingly.
// 6. Don't leave the tailing character.
// 7. Call "moveBufPtr(...)" if  needed.
//
// For  example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling deleteChar()---
//
// cmd> This is he command
//              ^
//
bool CmdParser::deleteChar() {
    char* cur = _readBufPtr;

    if (_readBufPtr != _readBufEnd) {
        // Replace the last char on the screen with space
        cout << "\rcmd> " << _readBuf << '\b' << ' ';

        // Move remain characters left
        while (cur != _readBufEnd) {
            *cur = *(cur + 1);
            cur++;
        }
        _readBufEnd--;
        *_readBufEnd = 0;

        // Cursor scrolling back from the end of _readBuf[]
        cur = _readBufEnd;
        cout << "\rcmd> " << _readBuf;
        while ((cur--) != _readBufPtr) cout << '\b';    
    }
    else {
        mybeep();
        return false;
    }
    return true;
}

bool CmdParser::backSpaceChar() {
    char* cur = _readBufPtr;

    if (cur != _readBuf) {
        cout << "\rcmd> " << _readBuf << '\b' << ' ';

        while (cur != _readBufEnd) {
            *(cur-1) = *cur; // Difference from delete
            cur++;
        }
        _readBufPtr--; // Difference from delete
        _readBufEnd--;
        *_readBufEnd = 0;

        cur = _readBufEnd;
        cout << "\rcmd> " << _readBuf;
        while ((cur--) != _readBufPtr) cout << '\b';
    }
    else  {
        mybeep();
        return false;
    }
    return true;
}

// 1. Insert character 'ch' for  "repeat" times at _readBufPtr
// 2. Move the remaining string right for  "repeat" characters
// 3. The cursor should move right for  "repeats" positions afterwards
// 4. Default value for  "repeat" is 1. You should assert that (repeat >= 1).
//
// For  example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling insertChar('k', 3) ---
//
// cmd> This is kkkthe command
//                 ^
//
void CmdParser::insertChar(char ch, int repeat) {
    string temp(_readBufPtr);

    for (int i = 0 ; i < repeat ; i++) {
        *_readBufPtr = ch;
        _readBufPtr++;
    }

    if (temp.size() != 0)
        strcpy(_readBufPtr,temp.c_str());

    _readBufEnd += repeat;
    *_readBufEnd = 0; //Make sure it points to '\0'

    char* cur = _readBufEnd;
    cout << "\rcmd> " << _readBuf;
    while ((cur--) != _readBufPtr) cout << '\b';

    assert(repeat >= 1);
}

// 1. Delete the line that is currently shown on the screen
// 2. Reset _readBufPtr and _readBufEnd to _readBuf
// 3. Make sure *_readBufEnd = 0
//
// For  example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling deleteLine() ---
//
// cmd>
//      ^
//
void CmdParser::deleteLine() {
    char* cur = _readBuf;
    cout << '\r';
    while (cur != _readBufEnd + 5) {
        cout << ' ';
        cur++;
    }
    _readBufPtr = _readBufEnd = _readBuf;
    *_readBufEnd = 0;
}


// This functions moves _historyIdx to index and display _history[index]
// on the screen.
//
// Need to consider:
// If  moving up... (i.e. index < _historyIdx)
// 1. If  already at top (i.e. _historyIdx == 0), beep and do nothing.
// 2. If  at bottom, temporarily record _readBuf to history.
//    (Do not remove spaces, and set _tempCmdStored to "true")
// 3. If  index < 0, let index = 0.
//
// If  moving down... (i.e. index > _historyIdx)
// 1. If  already at bottom, beep and do nothing
// 2. If  index >= _history.size(), let index = _history.size() - 1.
//
// Assign _historyIdx to index at the end.
//
// [Note] index should not = _historyIdx
//
void CmdParser::moveToHistory(int index) {
    // Page UP || UP arrow
    if (index < _historyIdx) {
        if (index < 0) mybeep();
        else {
            if (index == (int)_history.size() - 1) { // Point to the last entry
                string temp(_readBuf);
                _history.push_back(temp);
                _tempCmdStored = true;
            }
            _historyIdx = index;
            retrieveHistory();
        }
    }
    // Page DW || DW arrow
    else  if (index > _historyIdx) {
        if (index >= (int)_history.size()) mybeep();
        else {
            _historyIdx = index;
            retrieveHistory();
            
            if (index == (int)_history.size() - 1 and _tempCmdStored) { // When go back to tempt history string, renew tempt string
                _history.pop_back(); 
                _tempCmdStored = false;
                _historyIdx = (int)_history.size(); // Since _history is modified, reset _historyIdx to the len of _history
            }
        }
    }
}
// This function adds the string in _readBuf to the _history.
// The size of _history may or may not change. Depending on whether 
// there is a temp history string.
//
// 1. Remove ' ' at the beginning and end of _readBuf
// 2. If  not a null string, add string to _history.
//    Be sure you are adding to the right entry of _history.
// 3. If  it is a null string, don't add anything to _history.
// 4. Make sure to clean up "temp recorded string" (added earlier by up/pgUp,
//    and reset _tempCmdStored to false
// 5. Reset _historyIdx to _history.size() // for  future insertion
//
void CmdParser::addHistory() {
    string buff;
    char *head = _readBuf, *tail = _readBufEnd;

    // Remove tempt history string
    if (_tempCmdStored == true) {
        _history.pop_back();
        _tempCmdStored = false;
    }

    // Exclude null strings 
    while (*head == ' ' and head != tail) head++;
    while (*(tail-1) == ' ' and head != tail) tail--;

    while (head != tail) buff += *(head++);
    if (buff != "\0") _history.push_back(buff);

    _historyIdx = (int)_history.size(); // Reset when pressing enter

    //printHistory();
}


// 1. Replace current line with _history[_historyIdx] on the screen
// 2. Set _readBufPtr and _readBufEnd to end of line
//
// [Note] Do not change _history.size().
//
void CmdParser::retrieveHistory() {
   deleteLine();
   strcpy(_readBuf, _history[_historyIdx].c_str());
   cout << "\rcmd> "<<_readBuf;
   _readBufPtr = _readBufEnd = _readBuf + _history[_historyIdx].size();
}
/* Auxilliary function - Debug only */
/*
void CmdParser::printHistory() {
    cout << "\n_history: {" ;
    for (int i=0 ; i < (int)_history.size() ; i++)
        cout << "\""<<_history[i] << "\",";
    cout << "}";
}
*/