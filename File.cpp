//
// Created by kaprisa on 22.03.18.
//

#include "File.h"


map<std::string, int> File::words;

void File::read(string const &name, void cb(ifstream&)) {
    ifstream f(name);
    if (f.is_open()) {
        cb(f);
        f.close();
    } else {
        cout << "Не удалось открыть файл " << name << endl;
    }
}

void File::write(string const &name, string const &text) {
    ofstream file;
    file.open (name);
    file << text;
    file.close();
}
