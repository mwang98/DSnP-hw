/****************************************************************************
  FileName     [ test.cpp ]
  PackageName  [ test ]
  Synopsis     [ Test program for simple database db ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include "dbJson.h"

using namespace std;

extern DBJson dbjson;

class CmdParser;
CmdParser* cmdMgr = 0; // for linking purpose

int
main(int argc, char** argv)
{
   if (argc != 2) {  // testdb <jsonfile>
      cerr << "Error: using testdb <jsonfile>!!" << endl;
      exit(-1);
   }

   ifstream inf(argv[1]);

   if (!inf) {
      cerr << "Error: cannot open file \"" << argv[1] << "\"!!\n";
      exit(-1);
   }

   if (dbjson) {
      cout << "Table is resetting..." << endl;
      dbjson.reset();
   }
   if (!(inf >> dbjson)) {
      //cerr << "Error in reading JSON file!!" << endl;
      //exit(-1);
   }

   cout << "========================" << endl;
   cout << " Print JSON object" << endl;
   cout << "========================" << endl;
   cout << dbjson << endl;

   size_t a = 0;
   cout << "Maximun Element: " << dbjson.max(a) << endl;
   cout << "Minimun Element: " << dbjson.min(a) << endl;
   cout << "Average: " << dbjson.ave() << endl;
   cout << "Sum: " << dbjson.sum() << endl;
   cout << "Add: mike 1026" << endl;
   dbjson.add(DBJsonElem(string("mike"),1026));
   cout << dbjson << endl;
   
   return 0;
}
