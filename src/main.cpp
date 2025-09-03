//
// Created by dioguabo-rei-delas on 8/13/25.
#include "../libs/frameWork/core.h"
#include "../libs/frameWork/dynamicType/lazyAny.h"

using namespace std;

struct fool {
    double i[128];
};

int main(int argc, const char** argv) {

    const auto buffer = cid::help::openFile(argc, argv);
    const auto tokens = cid::tok::Tokenizer(buffer).tokenize();
    const auto code = cid::code::unsafePrototypeCode(tokens);
    //cid::tok::printToken(tokens);
    return cid::code::unsafeRun(code);


}
