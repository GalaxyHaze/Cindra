//
// Created by dioguabo-rei-delas on 8/13/25.
#include "../libs/frameWork/core.h"
#include "../libs/frameWork/virtualMachine/code.h"

using namespace std;
void foo(int i) {
    cout << "What hell happened? " << i << endl;
}
int main(int argc, const char** argv) {

    const auto buffer = cid::help::openFile(argc, argv);
    const auto tokens = cid::tok::Tokenizer(buffer).tokenize();
    const auto code = cid::code::generateByteCode(tokens);
    cid::code::safeRun(code);

    return 0;
}