//
// Created by weiyanguo on 2021/12/27.
//

#include "io_affairs.h"


int main(){
    ifstream inf;
    ofstream of;

    vector<ulong64> hl = {123,1243245,576657};
    string st = {"Hellohelkafdkaslj\n"};

    of.open("test.out", ios::binary| ios::trunc);
    save_hash_list(hl, of);
    save_cstr((char*)st.c_str(), of);
    of.close();

    inf.open("test.out", ios::binary);
    vector<ulong64> hlll;
    string stt;
    read_hash_list(inf, hlll);
    stt = read_cstr(inf);

    cout << stt << endl << hlll[2] << endl << hlll[1];
}
