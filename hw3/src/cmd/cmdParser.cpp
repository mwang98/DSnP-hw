/****************************************************************************
  FileName     [ cmdParser.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command parsing member functions for class CmdParser ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <string>
#include <vector>
#include "util.h"
#include "cmdParser.h"

using namespace std;

//----------------------------------------------------------------------
//    External funcitons
//----------------------------------------------------------------------
void mybeep();


//----------------------------------------------------------------------
//    Member Function for class cmdParser
//----------------------------------------------------------------------
// return false if file cannot be opened
// Please refer to the comments in "DofileCmd::exec", cmdCommon.cpp
bool CmdParser::openDofile(const string& dof){
    // TODO...
    if(_dofileStack.size() > 1024){
        cerr << "Error: dofile stack overflow (1024)\n";
        return false;
    }
    #ifdef __APPLE__
    if(_dofileStack.size() > 252){
        cerr << "Error: dofile stack overflow (252)\n";
        return false;
    }
    #endif

    _dofile = new ifstream(dof.c_str());

    if(!(_dofile -> is_open())){
        delete _dofile;
        if(_dofileStack.empty()) _dofile = 0;
        else _dofile = _dofileStack.top();
        return false;
    }
    _dofileStack.push(_dofile);

return true;
}

// Must make sure _dofile != 0
void CmdParser::closeDofile(){
    assert(_dofile != 0);
    // TODO...
    _dofile -> close();
    _dofileStack.pop();
    delete _dofile;
    // after finishing new dofile, retrieve _dofile.
    if(_dofileStack.empty()) _dofile = 0;
    else _dofile = _dofileStack.top();
}

// Return false if registration fails
bool
CmdParser::regCmd(const string& cmd, unsigned nCmp, CmdExec* e)
{
   // Make sure cmd hasn't been registered and won't cause ambiguity
   string str = cmd;
   unsigned s = str.size();
   if (s < nCmp) return false;
   while (true) {
      if (getCmd(str)) return false;
      if (s == nCmp) break;
      str.resize(--s);
   }

   // Change the first nCmp characters to upper case to facilitate
   //    case-insensitive comparison later.
   // The strings stored in _cmdMap are all upper case
   //
   assert(str.size() == nCmp);  // str is now mandCmd
   string& mandCmd = str;
   for (unsigned i = 0; i < nCmp; ++i)
      mandCmd[i] = toupper(mandCmd[i]);
   string optCmd = cmd.substr(nCmp);
   assert(e != 0);
   e->setOptCmd(optCmd);

   // insert (mandCmd, e) to _cmdMap; return false if insertion fails.
   return (_cmdMap.insert(CmdRegPair(mandCmd, e))).second;
}

// Return false on "quit" or if excetion happens
CmdExecStatus
CmdParser::execOneCmd()
{
   bool newCmd = false;
   if (_dofile != 0)
      newCmd = readCmd(*_dofile);
   else
      newCmd = readCmd(cin);

   // execute the command
   if (newCmd) {
      string option;
      CmdExec* e = parseCmd(option);
      if (e != 0)
         return e->exec(option);
   }
   return CMD_EXEC_NOP;
}

// For each CmdExec* in _cmdMap, call its "help()" to print out the help msg.
// Print an endl at the end.
void CmdParser::printHelps() const{
    // TODO...
    CmdMap::const_iterator it = _cmdMap.begin();
    while(it != _cmdMap.end()) (it++) -> second -> help(); 
    cout << endl;
}

void
CmdParser::printHistory(int nPrint) const
{
   assert(_tempCmdStored == false);
   if (_history.empty()) {
      cout << "Empty command history!!" << endl;
      return;
   }
   int s = _history.size();
   if ((nPrint < 0) || (nPrint > s))
      nPrint = s;
   for (int i = s - nPrint; i < s; ++i)
      cout << "   " << i << ": " << _history[i] << endl;
}


//
// Parse the command from _history.back();
// Let string str = _history.back();
//
// 1. Read the command string (may contain multiple words) from the leading
//    part of str (i.e. the first word) and retrive the corresponding
//    CmdExec* from _cmdMap
//    ==> If command not found, print to cerr the following message:
//        Illegal command!! "(string cmdName)"
//    ==> return it at the end.
// 2. Call getCmd(cmd) to retrieve command from _cmdMap.
//    "cmd" is the first word of "str".
// 3. Get the command options from the trailing part of str (i.e. second
//    words and beyond) and store them in "option"
//
CmdExec* CmdParser::parseCmd(string& option){
    assert(_tempCmdStored == false);
    assert(!_history.empty());
    string str = _history.back();
    string cmd;
    CmdExec* e;
    assert(str[0] != 0 && str[0] != ' ');

    myStrGetTok(str,cmd);
    option = str.substr(cmd.size(),string::npos);
    if(option.size()) while(option[0] == ' ')option.erase(0,1);
    e = getCmd(cmd);

    if(!e) cerr << "Illegal command!! (" << cmd << ")" << endl;
    return e;
    // TODO...
}

// Remove this function for TODO...
//
// This function is called by pressing 'Tab'.
// It is to list the partially matched commands.
// "str" is the partial string before current cursor position. It can be 
// a null string, or begin with ' '. The beginning ' ' will be ignored.
//
// Several possibilities after pressing 'Tab'
// (Let $ be the cursor position)
// 1. LIST ALL COMMANDS
//    --- 1.1 ---
//    [Before] Null cmd
//    cmd> $
//    --- 1.2 ---
//    [Before] Cmd with ' ' only
//    cmd>     $
//    [After Tab]
//    ==> List all the commands, each command is printed out by:
//           cout << setw(12) << left << cmd;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location (including ' ')
//
// 2. LIST ALL PARTIALLY MATCHED COMMANDS
//    --- 2.1 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$                   // partially matched
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$                   // and then re-print the partial command
//    --- 2.2 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$llo                // partially matched with trailing characters
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$llo                // and then re-print the partial command
//
// 3. LIST THE SINGLY MATCHED COMMAND
//    ==> In either of the following cases, print out cmd + ' '
//    ==> and reset _tabPressCount to 0
//    --- 3.1 ---
//    [Before] partially matched (single match)
//    cmd> he$
//    [After Tab]
//    cmd> heLp $               // auto completed with a space inserted
//    --- 3.2 ---
//    [Before] partially matched with trailing characters (single match)
//    cmd> he$ahah
//    [After Tab]
//    cmd> heLp $ahaha
//    ==> Automatically complete on the same line
//    ==> The auto-expanded part follow the strings stored in cmd map and
//        cmd->_optCmd. Insert a space after "heLp"
//    --- 3.3 ---
//    [Before] fully matched (cursor right behind cmd)
//    cmd> hElP$sdf
//    [After Tab]
//    cmd> hElP $sdf            // a space character is inserted
//
// 4. NO MATCH IN FITST WORD
//    --- 4.1 ---
//    [Before] No match
//    cmd> hek$
//    [After Tab]
//    ==> Beep and stay in the same location
//
// 5. FIRST WORD ALREADY MATCHED ON FIRST TAB PRESSING
//    --- 5.1 ---
//    [Before] Already matched on first tab pressing
//    cmd> help asd$gh
//    [After] Print out the usage for the already matched command
//    Usage: HELp [(string cmd)]
//    cmd> help asd$gh
//
// 6. FIRST WORD ALREADY MATCHED ON SECOND AND LATER TAB PRESSING
//    ==> Note: command usage has been printed under first tab press
//    ==> Check the word the cursor is at; get the prefix before the cursor
//    ==> So, this is to list the file names under current directory that
//        match the prefix
//    ==> List all the matched file names alphabetically by:
//           cout << setw(16) << left << fileName;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location
//    --- 6.1 ---
//    [Before] if prefix is empty, print all the file names
//    cmd> help $sdfgh
//    [After]
//    .               ..              Homework_3.docx Homework_3.pdf  Makefile
//    MustExist.txt   MustRemove.txt  bin             dofiles         include
//    lib             mydb            ref             src             testdb
//    cmd> help $sdfgh
//    --- 6.2 ---
//    [Before] with a prefix and with mutiple matched files
//    cmd> help M$Donald
//    [After]
//    Makefile        MustExist.txt   MustRemove.txt
//    cmd> help M$Donald
//    --- 6.3 ---
//    [Before] with a prefix and with mutiple matched files,
//             and these matched files have a common prefix
//    cmd> help Mu$k
//    [After]
//    ==> auto insert the common prefix and make a beep sound
//    ==> DO NOT print the matched files
//    cmd> help Must$k
//    --- 6.4 ---
//    [Before] with a prefix and with a singly matched file
//    cmd> help MustE$aa
//    [After] insert the remaining of the matched file name followed by a ' '
//    cmd> help MustExist.txt $aa
//    --- 6.5 ---
//    [Before] with a prefix and NO matched file
//    cmd> help Ye$kk
//    [After] beep and stay in the same location
//    cmd> help Ye$kk
//
//    [Note] The counting of tab press is reset after "newline" is entered.
//
// 7. FIRST WORD NO MATCH
//    --- 7.1 ---
//    [Before] Cursor NOT on the first word and NOT matched command
//    cmd> he haha$kk
//    [After Tab]
//    ==> Beep and stay in the same location

void
CmdParser::listCmd(const string& str){
    // Null OR Space string
    size_t i = 0;
    bool emptyORspace = true;
    CmdMap::iterator it = _cmdMap.begin();
    for(; i < str.size() ; i++) if(str[i] != ' ') emptyORspace = false;
    if(emptyORspace){
        cout << endl;
        i = 1;
        while(it != _cmdMap.end()){
            string combine = it -> first + it -> second -> getOptCmd();
            cout << setw(12) << left << combine;
            if(i%5 == 0) cout << endl;
            i++; 
            it++;
        }
        reprintCmd();
        return;
    }

    bool already_match = false, cmdIsB4cur = false;
    string token, substr, fileName;                 // token    : substring before the first space
                                                    // substr   : substring after the last space
                                                    // fileName : substring between the last and the previous of the last space
    vector<string> partially_match;                 // include already-matched as well 
    vector<string> files;                           // all of the files under the directory
    CmdExec* e;

    // Distract substrings from str
    substr = str.substr((str.find_last_of(' ')+1));
    myStrGetTok(str,token); 
    if(token != str){
        fileName = str;
        auto it_f = fileName.end(); it_f--;
        while(true){
            if(*it_f != ' ') break;
            fileName.erase(it_f--);
        }
        fileName = fileName.substr((fileName.find_last_of(' ')+1));
    }

    if(token == str) cmdIsB4cur = true;
    e = getCmd(token);

    it = _cmdMap.begin();
    // Token is partially matched which cmds?
    while(it != _cmdMap.end()){
        string combine = it -> first + it -> second -> getOptCmd();
        if(token.size() > combine.size());
        // Check partially matched
        else if(myStrNCmp(combine, token, token.size()) == 0){
            partially_match.push_back(combine);

            // Check whether already-matched
            if(myStrNCmp(combine, token, (it -> first).size()) == 0)
                already_match = true;
        }
        it++;
    }

    if(already_match){
        if(cmdIsB4cur){
            // Fully matched -> autofill with space
            if(partially_match[0].size() == token.size() and \
                myStrNCmp(partially_match[0], token, token.size()) == 0)
                insertChar(' '); 
            // Not fully matched -> autofill and make it fully matched
            else{
                string prnt = partially_match[0].substr(token.size(),partially_match[0].size()-token.size());
                for(i=0 ; i<prnt.size(); i++) insertChar(prnt[i]); 
                insertChar(' ');
            }
            _tabPressCount = 0;
        }
        // Cmd isn't directly before cursor [cmd $321]
        else{
            // Print [Usage]
            if(_tabPressCount == 1){
                cout << endl; e -> usage(cout);
                reprintCmd();
            }
            // Print files under the directory
            else{
                listDir(files,"",".");
                
                // Determine whether files under the dir have the common prefix
                size_t cnt = 0, pos = 0;
                bool hasCommon = false;
                string common_prefix;
                common_prefix += files[0][0];

                do{
                    char cmp = files[0][pos];
                    cnt = 0;
                    for(i=0 ; i<files.size() ; i++) if(files[i][pos] == cmp) cnt++;
                    if(cnt == files.size() and pos > 0){
                        common_prefix += cmp;
                        hasCommon = true;
                    }  
                    pos++;
                    if(common_prefix.size() == files[0].size()) break;
                } while(cnt == files.size());
                
                // Case1: At least one space before cursor => prefix = NULL
                if(substr == ""){
                    // Only one file under the dir -> autofill the file name
                    if(files.size() == 1){
                        if(fileName != files[0]){
                            for(i=0 ; i<files[0].size(); i++) insertChar(files[0][i]);
                            insertChar(' ');
                        }
                         mybeep();
                    }
                    // NO common prefix -> print out all files under dir
                    else if(!hasCommon){
                        cout << endl;
                        for(i=0 ; i<files.size() ; i++){
                            cout << setw(16) << left << files[i];
                            if(i%5 == 4) cout << endl;
                        }
                        reprintCmd();
                    }
                    // Do have common prefix -> autofill common_prefix
                    else{
                        for(i=0 ; i<common_prefix.size() ; i++)
                            insertChar(common_prefix[i]);
                        mybeep();
                    }
                }
                // Case2: NO space B4 cursor => treated substring as prefix
                else{
                    // Modify vector<string> files. Treated substr as prefix.
                    files.clear();
                    listDir(files, substr, ".");

                    // NO files match substr
                    if(files.size() == 0) mybeep();
                    // More than one files match substr
                    else{
                        // Renew common_prefix since files has changed
                        common_prefix.clear();
                        common_prefix += substr;
                        pos = substr.size()-1;

                        do{
                            char cmp = files[0][pos];
                            cnt = 0;
                            for(i=0 ; i<files.size() ; i++) if(files[i][pos] == cmp) cnt++;
                            if(cnt == files.size() and pos > substr.size()-1) common_prefix += cmp;   
                            pos++;
                            if(common_prefix.size() == files[0].size()) break;
                        } while(cnt == files.size());
                        
                        // substr is also substring of common_prefix -> autofill common_prefix
                        if(substr < common_prefix or files.size() == 1){
                            string prnt = common_prefix.substr(substr.size(),common_prefix.size()-substr.size());
                            for(i=0 ; i<prnt.size(); i++) insertChar(prnt[i]);
                            if(files.size() == 1) insertChar(' ');
                            mybeep();
                        }
                        // NO common_prefix OR substr is more specified, or euqally specified
                        else{                      
                            cout << endl;
                            for(i=0 ; i<files.size() ; i++){
                                cout << setw(16) << left << files[i];
                                if(i%5 == 4) cout << endl;
                            }
                            reprintCmd();
                        }
                    }
                }
            }
        }
    }
    // Not already matched
    else{
        if(!cmdIsB4cur){ mybeep(); return; }
        // NO matched file
        if(partially_match.size() == 0) mybeep();
        // Match exactly one cmd
        else if(partially_match.size() == 1){
            string prnt = partially_match[0].substr(str.size(),partially_match[0].size()-str.size());
            for(i=0 ; i<prnt.size(); i++) insertChar(prnt[i]);
            insertChar(' ');
            _tabPressCount = 0;
        }
        // More than two cmds are matched
        else{
            cout << endl;
            for(i=0 ; i<partially_match.size() ; i++){
                cout << setw(12) << left << partially_match[i];
                if(i%5 == 4) cout << endl;
            }
            reprintCmd();
        }
    }
}

// cmd is a copy of the original input
//
// return the corresponding CmdExec* if "cmd" matches any command in _cmdMap
// return 0 if not found.
//
// Please note:
// ------------
// 1. The mandatory part of the command string (stored in _cmdMap) must match
// 2. The optional part can be partially omitted.
// 3. All string comparison are "case-insensitive".
//
CmdExec* CmdParser::getCmd(string cmd){
    CmdExec* e = 0;
    // TODO...
    CmdMap::const_iterator it = _cmdMap.begin();
    while(it != _cmdMap.end()){
        string std_cmd = (it -> first) + (it -> second -> getOptCmd());
        if(std_cmd.size() < cmd.size());
        else if(myStrNCmp(std_cmd, cmd, (it -> first).size()) == 0){
            e = it -> second;
            break;
        }
        it++;
    }
    return e;
}


//----------------------------------------------------------------------
//    Member Function for class CmdExec
//----------------------------------------------------------------------
// return false if option contains an token
bool
CmdExec::lexNoOption(const string& option) const
{
   string err;
   myStrGetTok(option, err);
   if (err.size()) {
      errorOption(CMD_OPT_EXTRA, err);
      return false;
   }
   return true;
}

// Return false if error options found
// "optional" = true if the option is optional XD
// "optional": default = true
//
bool
CmdExec::lexSingleOption
(const string& option, string& token, bool optional) const
{
   size_t n = myStrGetTok(option, token);
   if (!optional) {
      if (token.size() == 0) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
   }
   if (n != string::npos) {
      errorOption(CMD_OPT_EXTRA, option.substr(n));
      return false;
   }
   return true;
}

// if nOpts is specified (!= 0), the number of tokens must be exactly = nOpts
// Otherwise, return false.
//
bool
CmdExec::lexOptions
(const string& option, vector<string>& tokens, size_t nOpts) const
{
   string token;
   size_t n = myStrGetTok(option, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(option, token, n);
   }
   if (nOpts != 0) {
      if (tokens.size() < nOpts) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
      if (tokens.size() > nOpts) {
         errorOption(CMD_OPT_EXTRA, tokens[nOpts]);
         return false;
      }
   }
   return true;
}

CmdExecStatus
CmdExec::errorOption(CmdOptionError err, const string& opt) const
{
   switch (err) {
      case CMD_OPT_MISSING:
         cerr << "Error: Missing option";
         if (opt.size()) cerr << " after (" << opt << ")";
         cerr << "!!" << endl;
      break;
      case CMD_OPT_EXTRA:
         cerr << "Error: Extra option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_ILLEGAL:
         cerr << "Error: Illegal option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_FOPEN_FAIL:
         cerr << "Error: cannot open file \"" << opt << "\"!!" << endl;
      break;
      default:
         cerr << "Error: Unknown option error type!! (" << err << ")" << endl;
      exit(-1);
   }
   return CMD_EXEC_ERROR;
}

