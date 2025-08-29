//
// Created by dioguabo-rei-delas on 8/13/25.
#include "../libs/frameWork/core.h"

using namespace std;
void foo(int i) {
    cout << "What hell happened? " << i << endl;
}
int main(int argc, const char** argv) {
    /*
    const auto buffer = cid::help::openFile(argc, argv);
    const auto tokens = cid::tok::Tokenizer(buffer).tokenize();
    printToken(tokens);*/
    const void* ptr = (void*)&foo;
    goto *ptr;
    return 0;
}