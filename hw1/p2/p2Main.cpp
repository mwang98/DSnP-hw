/****************************************************************************
  FileName     [ p2Main.cpp ]
  PackageName  [ p2 ]
  Synopsis     [ Define main() function ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2016-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <string>
#include <regex>
#include <iomanip>
#include "p2Json.h"

using namespace std;

int main()
{
    Json json;

    // Read in the csv file. Do NOT change this part of code.
     string jsonFile;
    cout << "Please enter the file name: ";
    cin >> jsonFile;
    if (json.read(jsonFile))
        cout << "File \"" << jsonFile << "\" was read in successfully." << endl;
    else {
        cerr << "Failed to read in file \"" << jsonFile << "\"!" << endl;
        exit(-1); // jsonFile does not exist.
    }

   // TODO read and execute commands
   
    while (true) {
         string command;
         cout << "Enter command: ";
         cin >> command;
         if(command == "PRINT") json.print();
         else if(command == "SUM"){
            if(json.is_empty());
            else
              cout << "The summation of the values is: " << json.sum() << ".\n";
         }
         else if(command == "AVE"){
            if(json.is_empty());
            else
              cout << "The average of the values is: " << fixed << setprecision(1) << json.ave() << ".\n";
         }
         else if(command == "MAX"){
            if(json.is_empty());
            else
              cout << "The maximum element is: { " << json.max() << " }.\n";
         }
         else if(command == "MIN"){
            if(json.is_empty());
            else
              cout << "The minimum element is: { " << json.min() << " }.\n";
         }
         else if(command == "EXIT") break;
         else if(command == "ADD"){
            string _key, _value;
            cin >> _key >> _value;
            regex pat_key("^\\w+$"),pat_val("^[^a-zA-Z]+$");

            if(regex_match(_key, pat_key) and regex_match(_value, pat_val))
                json.add(_key, stoi(_value));
        }
    }
     
}
