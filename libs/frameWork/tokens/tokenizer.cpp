#include "tokenizer.h"

namespace emb::tok {
    using namespace help;
    // -------- Token --------
    Token::Token():
    type(INVALID),
    data(),
    lexeme("INVALID"),
    line(0), column(0) {}

    Token::Token(TokenType type, std::string lexeme, std::vector<std::byte> data,
                 size_t line, size_t column):
        type(type),
        data(std::move(data)),
        lexeme(std::move(lexeme)),
        line(line), column(column) {}


    // -------- Tokenizer --------
    Tokenizer::Tokenizer(std::string sourceFile)
        : input(std::move(sourceFile)) {
        tokens.reserve(100);
    }

    bool Tokenizer::hasToken() const noexcept {
        return current < input.size() && !input.empty();
    }

    char Tokenizer::peek() const noexcept {
        return (current < input.size()) ? input[current] : '\0';
    }

    char Tokenizer::peek(size_t i) const noexcept {
        return (i + current < input.size()) ? input[current + i] : '\0';
    }

    char Tokenizer::behind() const noexcept {
        return (current > 0) ? input[current - 1] : '\0';
    }

    char Tokenizer::behind(size_t i) const noexcept {
        return (current > i) ? input[current - i] : '\0';
    }

    char Tokenizer::next() noexcept {
        const char c = input[current++];
        if (c == '\n') {
            ++line;
            column = 0;
        } else {
            ++column;
        }
        return c;
    }

    char Tokenizer::next(size_t i) noexcept {
        if (i + current < input.size()) {
            current += i;
            return input[current];
        }
        return '\0';
    }

    void Tokenizer::skipLine() {
        while (hasToken() && peek() != '\n' && peek() != '\r') {
            next();
        }
    }

    void Tokenizer::digitProcess() {
        std::string token;
        token += behind();
        const size_t collum = column;
        while (hasToken() && isDigit(peek())) {
            token += next();
        }
        tokens.emplace_back(NUMBER_LITERAL, token, to_byte(to_digit(token)), line, collum);
    }

    void Tokenizer::tokenize() {
        while (hasToken()) {
            const auto c = next();
            if (isSpace(c))
                continue;
            if (isDigit(c)) {
                digitProcess();
                continue;
            }
            if (c == '/' && peek() == '/') {
                skipLine();
                continue;
            }
        }
    }

} // namespace emb
