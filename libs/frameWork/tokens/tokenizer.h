//
// tokenizer.h
// Header-only Tokenizer para Cindra
//
#ifndef CINDRA_TOKENIZER_H
#define CINDRA_TOKENIZER_H
#include <string>
#include <vector>
#include <stdexcept>
#include <cstddef>
#include "helper.h"
#include <iostream>
#include "../containers/unordered_dense_map.h"
using std::vector;
using std::string;
using std::cout;

namespace cid::tok {
    using namespace help;

    enum TokenType: uint8_t {
        INVALID,
        KEYWORD,
        IDENTIFIER,
        INT_LITERAL,
        STRING_LITERAL,
        PRINT,
        RETURN,
        SEMICOLON
    };

    inline auto setUpkeywords() {
        ankerl::unordered_dense::map<std::string, cid::tok::TokenType> tmp;
        tmp["return"] = TokenType::RETURN;
        tmp["print"] = TokenType::PRINT;
        return tmp;
    }
    inline auto Keyword = setUpkeywords();

    struct Token {
        TokenType type;
        std::vector<std::byte> data;
        std::string lexeme;
        size_t line;
        size_t column;

        Token() : type(INVALID), data(), lexeme("INVALID"), line(0), column(0) {}
        Token(TokenType type, std::string lexeme, std::vector<std::byte> data,
              size_t line, size_t column)
            : type(type), data(std::move(data)), lexeme(std::move(lexeme)),
              line(line), column(column) {}
    };

    class Tokenizer {
        std::vector<Token> tokens;
        const std::string input;
        size_t current = 0;
        size_t line = 1, column = 1; // Começa na linha 1

        [[nodiscard]] bool hasToken() const noexcept {
            return current < input.size();
        }
        [[nodiscard]] char peek(size_t i = 0) const noexcept {
            return (current + i < input.size()) ? input[current + i] : '\0';
        }
        char next() noexcept {
            const char c = input[current++];
            if (c == '\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }
            return c;
        }
        void skipLine() {
            while (hasToken() && peek() != '\n') {
                next();
            }
            if (hasToken()) next(); // Consome o '\n'
        }
        void skipMultiline() {
            while (hasToken()) {
                if (peek() == '*' && peek(1) == '/') {
                    next(); next();
                    return;
                }
                next();
            }
            throw std::runtime_error("error in tokenizer: unterminated multi-line comment");
        }
        void intProcess(char firstDigit) {
            std::string token(1, firstDigit);
            const size_t col = column - 1; // Coluna correta
            while (hasToken() && isDigit(peek())) {
                token += next();
            }
            try {
                int value = std::stoi(token);
                tokens.emplace_back(INT_LITERAL, token, to_byte(value), line, col);
            } catch (...) {
                throw std::runtime_error("error in tokenizer: integer literal too large");
            }
        }
    public:
        explicit Tokenizer(std::string source)
            : input(std::move(source)) {
            tokens.reserve(100);
        }
        void strProcess(const char c) {
            std::string token(1, c); // Inclui a aspa inicial
            const size_t col = column - 1;

            while (hasToken() && peek() != '"') {
                token += next();
            }

            if (!hasToken()) {
                throw std::runtime_error("error in tokenizer: unterminated string");
            }

            token += next(); // Adiciona a aspa final
            tokens.emplace_back(STRING_LITERAL, token, to_byte(token), line, col);
        }
        void identProcess(char c) {
            std::string token(1, c);
            const size_t col = column - 1;

            while (hasToken()) {
                char n = peek();
                if (!(isAlpha(n) || isDigit(n) || isUnderscore(n))) {
                    break;
                }
                token += next();
            }

            auto it = Keyword.find(token);
            if (it != Keyword.end()) {
                tokens.emplace_back(it->second, token, to_byte(token), line, col);
            } else {
                throw std::runtime_error("error in tokenizer: identifiers aren't allowed for now");
            }
        }
        auto tokenize() {
            if (input.empty()) return std::vector<Token>();

            while (hasToken()) {
                char c = next();

                if (isSpace(c)) continue;

                if (isDigit(c)) {
                    intProcess(c);
                    continue;
                }

                if (c == '/' && peek() == '/') {
                    skipLine();
                    continue;
                }

                if (c == '/' && peek() == '*') {
                    next();
                    skipMultiline();
                    continue;
                }

                if (c == '"') {
                    strProcess(c);
                    continue;
                }

                if (isAlpha(c) || c == '_') {
                    identProcess(c);
                    continue;
                }

                if (c == ';') {
                    tokens.emplace_back(SEMICOLON, ";", to_byte(0x0), line, column - 1);
                    continue;
                }
            }
            return tokens;
        }
        [[nodiscard]] const std::vector<Token>& getTokens() const noexcept {
            return tokens;
        }
    };

    inline void printToken(const vector<cid::tok::Token>& tokens) {
        for (const auto& token : tokens) {
            cout << "Token: ";
            switch (token.type) {
                case cid::tok::PRINT:
                    cout << "PRINT";
                    break;
                case cid::tok::IDENTIFIER:
                    cout << "IDENTIFIER: " << token.lexeme;
                    break;
                case cid::tok::INT_LITERAL:
                    cout << "INT_LITERAL: " << token.lexeme;
                    break;
                case cid::tok::STRING_LITERAL:
                    cout << "STRING_LITERAL: " << token.lexeme;
                    break;
                case cid::tok::RETURN:
                    cout << "RETURN";
                    break;
                case cid::tok::SEMICOLON:
                    cout << "SEMICOLON";
                    break;
                default:
                    cout << "UNKNOWN";
                    break;
            }
            cout << " (Line: " << token.line << ", Column: " << token.column << ")\n";
        }
    }
}
#endif // CINDRA_TOKENIZER_H