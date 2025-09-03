//
// token_type.h - central TokenType enum
//
#ifndef CINDRA_TOKEN_TYPE_H
#define CINDRA_TOKEN_TYPE_H
#include <cstdint>

namespace cid::tok {
    enum TokenType : uint8_t {
        INVALID,
        KEYWORD,
        IDENTIFIER,
        INT_LITERAL,
        STRING_LITERAL,
        PRINT,
        RETURN,
        SEMICOLON
    };
}

#endif // CINDRA_TOKEN_TYPE_H

