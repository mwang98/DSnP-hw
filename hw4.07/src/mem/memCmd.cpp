/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include <utility>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

bool
initMemCmd()
{
   if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
         cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
         cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
         cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
      )) {
      cerr << "Registering \"mem\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))
      return CMD_EXEC_ERROR;
   if (token.size()) {
      int b;
      if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {
         cerr << "Illegal block size (" << token << ")!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
      }
      #ifdef MEM_MGR_H
      mtest.reset(toSizeT(b));
      #else
      mtest.reset();
      #endif // MEM_MGR_H
   }
   else
      mtest.reset();
   return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{  
   os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{  
   cout << setw(15) << left << "MTReset: " 
        << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
    // TODO
    // Use  to catch the bad_alloc exception
    int num = -1, arraySize = -1;
    bool isArray = false;
    vector<string> tokens;
    if(!CmdExec::lexOptions(option, tokens)) return CMD_EXEC_ERROR;

    for(size_t i = 0, n = tokens.size() ; i < n ; i++){
        if(myStrNCmp("-Array", tokens[i], 2) == 0){
            if(arraySize == -1){
                isArray = true;
                if(i + 1 < tokens.size()){
                    if(!myStr2Int(tokens[i+1], arraySize) or arraySize <= 0){
                        return errorOption(CMD_OPT_ILLEGAL, tokens[i+1]);
                    }
                    i++;
                }
                else return errorOption(CMD_OPT_MISSING, tokens[i]);
            }
            else return errorOption(CMD_OPT_EXTRA, tokens[i]);
        }
        else if(num == -1){
            if(!myStr2Int(tokens[i], num) or num <= 0)
                return errorOption(CMD_OPT_ILLEGAL, tokens[i]);
        }
        else return errorOption(CMD_OPT_EXTRA, tokens[i]);       
    }
    if(num == -1) return errorOption(CMD_OPT_MISSING,"");


    try{
        if(!isArray) mtest.newObjs(num);
        else mtest.newArrs(num, arraySize);
    }
    catch(const bad_alloc& bd){}
    return CMD_EXEC_DONE;
}

void
MTNewCmd::usage(ostream& os) const
{  
   os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{  
   cout << setw(15) << left << "MTNew: " 
        << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
   // TODO
    vector<string> tokens;
    int idx = -1, rnNum = -1;
    pair<string, string> parameter;
    bool isArray = false, isIdx = false;
    if(!CmdExec::lexOptions(option, tokens)) return CMD_EXEC_ERROR;

    for(size_t i = 0, n = tokens.size() ; i < n  ; i++){
        if(myStrNCmp("-Array", tokens[i], 2) == 0){
            if(isArray) return errorOption(CMD_OPT_EXTRA, tokens[i]);
            else isArray = true;
        }
        else if(myStrNCmp("-Index", tokens[i], 2) == 0){
            if(idx == -1 and rnNum == -1){
                if(i + 1 < tokens.size()){
                    parameter.first = tokens[i];
                    parameter.second = tokens[i+1];
                    if(!myStr2Int(tokens[i+1], idx) or idx < 0)
                        return errorOption(CMD_OPT_ILLEGAL, tokens[i+1]);                           
                }
                else return errorOption(CMD_OPT_MISSING, tokens[i]);
            }
            else return errorOption(CMD_OPT_EXTRA, tokens[i]);
            isIdx = true;
            i++;
        }
        else if(myStrNCmp("-Random", tokens[i], 2) == 0){
            if(idx == -1 and rnNum == -1){
                if(i + 1 < tokens.size()){ 
                    parameter.first = tokens[i];
                    parameter.second = tokens[i+1];
                    if(!myStr2Int(tokens[i+1], rnNum) or rnNum <= 0)
                        return errorOption(CMD_OPT_ILLEGAL, tokens[i+1]);
                }
                else return errorOption(CMD_OPT_MISSING, tokens[i]); 
            }
            else return errorOption(CMD_OPT_EXTRA, tokens[i]);
            i++;
        }
        else return errorOption(CMD_OPT_ILLEGAL, tokens[i]);
    }
    if(idx == -1 and rnNum == -1) return errorOption(CMD_OPT_MISSING,"");

    if(isArray){
        if(isIdx){
            if((size_t)idx >= mtest.getArrListSize()){
                cerr << "Size of array list (" << mtest.getArrListSize() << ") is <= " << idx << "!!\n";
                return errorOption(CMD_OPT_ILLEGAL, parameter.second);
            }
        }
        else{
            if(mtest.getArrListSize() == 0){
                cerr << "Size of array list is 0!!\n";
                return errorOption(CMD_OPT_ILLEGAL, parameter.first);
            }
        }
    }
    else{
        if(isIdx){
            if((size_t)idx >= mtest.getObjListSize()){
                cerr << "Size of object list (" << mtest.getObjListSize() << ") is <= " << parameter.second << "!!\n";
                return errorOption(CMD_OPT_ILLEGAL, parameter.second);
            }
        }
        else{
            if(mtest.getObjListSize() == 0){
                cerr << "Size of object list is 0!!\n";
                return errorOption(CMD_OPT_ILLEGAL, parameter.first);
            }
        }
    }
            
    if(isIdx) isArray ? mtest.deleteArr(idx) : mtest.deleteObj(idx);
    else
        for(int i = 0 ; i < rnNum ; i++)
            isArray ? \
            mtest.deleteArr(rnGen(mtest.getArrListSize())):\
            mtest.deleteObj(rnGen(mtest.getObjListSize()));


    return CMD_EXEC_DONE;
}

void
MTDeleteCmd::usage(ostream& os) const
{  
   os << "Usage: MTDelete <-Index (size_t objId) | "
      << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{  
   cout << setw(15) << left << "MTDelete: " 
        << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
   // check option
   if (option.size())
      return CmdExec::errorOption(CMD_OPT_EXTRA, option);
   mtest.print();

   return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{  
   os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{  
   cout << setw(15) << left << "MTPrint: " 
        << "(memory test) print memory manager info" << endl;
}


