//
// Created by kaprisa on 22.03.18.
//

#ifndef MY_FILE_H
#define MY_FILE_H

#include <fstream>
#include <iostream>
#include <map>

using namespace std;

class File
{
public:
    static map<std::string, int> words;

    static void read(string const& name, void cb(ifstream& cb));
    static void write(string const& name, string const& text);
};


#endif //MY_FILE_H
