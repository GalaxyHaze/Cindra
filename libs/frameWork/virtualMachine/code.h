//
// Created by dioguabo-rei-delas on 8/29/25.
//

#ifndef CINDRA_CODE_H
#define CINDRA_CODE_H
#include "../tokens/tokenizer.h"
#include "../parser/parser.h"

namespace cid::code {

    template<class A>
    inline void insertByte(vector<A>& a, const vector<std::byte>& b) {
        const size_t s = b.size();
        auto tmp = reinterpret_cast<const A*>(b.data());
        for (size_t i = 0; i < s; i++) {
            a.emplace_back(tmp[i]);
        }
    }

    class CODE {
        const vector<uint8_t> code{};
        explicit CODE(const vector<uint8_t>& code) : code(code) {}
        friend auto generateCode(const vector<tok::Token>& );
        friend int run(const CODE&);
    };
    //NOTE: that is only for tests, that is unsafe, and cannot be used,
    //although if you are confident that your code is truly safe, and do not have erros
    //you can enable this function, and test your code a lot of times more fast
    //Because in this verion, the 'compiller' won't do any safe / syntax test
    auto generateByteCode(const par::CindraParserTree &parser);
    inline auto generateCode(const vector<tok::Token>& src) {
        size_t i = 0;
        for (const auto& token : src) {
            i += token.data.size();
        }
        std::vector<uint8_t> code;
        code.reserve(i);
        for (const auto& token : src) {
            if ( token.type == tok::INT_LITERAL || token.type == tok::STRING_LITERAL )
                insertByte(code, token.data);
            else
                code.emplace_back(static_cast<uint8_t>(token.type));
        }
        if ( src.size() != i)
            throw std::runtime_error("Interesting, the code surpass the expected size");
        return CODE(code);
    }

    //the function is 'int' because that will be the return of the programm
    inline int run(const CODE& src){
        size_t i = 0;
        #define NEXT() goto *denseInstructions[static_cast<tok::TokenType>(code[i++])];

        auto OP_Instructions = {&&PRINT, &&RETURN};
        help::FunctionState<tok::TokenType, 512> denseInstructions({tok::TokenType::PRINT, tok::TokenType::RETURN}, OP_Instructions);

        if (denseInstructions.empty())
            goto END;
        else
            goto *denseInstructions[static_cast<tok::TokenType>(src.code[i])];

        PRINT:
            auto ptr = src.code.data();

        RETURN:
        END:
        return 0;
    }

}
#endif //CINDRA_CODE_H