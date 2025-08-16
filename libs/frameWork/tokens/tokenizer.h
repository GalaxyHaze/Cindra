//
// Created by dioguabo-rei-delas on 8/13/25.
//
#ifndef CINDRA_TOKENIZER_H
#define CINDRA_TOKENIZER_H

#include <string>
#include <vector>
#include "helper.h"

namespace emb::tok {

    enum TokenType {
        INVALID,
        IDENTIFIER,
        NUMBER_LITERAL,
        STRING_LITERAL,
        PRINT,
    };

    struct Token {
        TokenType type;
        std::vector<std::byte> data;
        std::string lexeme;
        size_t line;
        size_t column;

        explicit Token();
        explicit Token(TokenType type, std::string lexeme, std::vector<std::byte> data,
                       size_t line, size_t column);
    };

    class Tokenizer {
        std::vector<Token> tokens;
        const std::string input;
        size_t current = 0;
        size_t line = 0, column = 0;

        [[nodiscard]] bool hasToken() const noexcept;
        [[nodiscard]] char peek() const noexcept;
        [[nodiscard]] char peek(size_t i) const noexcept;
        [[nodiscard]] char behind() const noexcept;
        [[nodiscard]] char behind(size_t i) const noexcept;
        char next() noexcept;
        char next(size_t i) noexcept;

        void skipLine();
        void digitProcess();

    public:
        explicit Tokenizer(std::string sourceFile);
        void tokenize();
    };

} // namespace emb

#endif //CINDRA_TOKENIZER_H
