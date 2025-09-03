//
// Created by dioguabo-rei-delas on 8/29/25.
//

#ifndef CINDRA_PARSER_H
#define CINDRA_PARSER_H
#include <string>
#include <vector>
#include <cstring>
#include "../tokens/token_type.h"
#include "../tokens/tokenizer.h"

namespace cid::par {
    // Intenção: validar apenas a sequência de tokens (modelo "assembly"),
    // sem construir AST, apenas checando forma e operandos mínimos.

    // Gramática mínima suportada:
    //   Stmt := PRINT INT_LITERAL ';'
    //         | PRINT STRING_LITERAL ';'
    //         | RETURN INT_LITERAL ';'
    //         | ';'                      // permitido como no-op
    //   Program := { Stmt }

    inline bool validateProgram(const std::vector<cid::tok::Token>& toks, std::string* err = nullptr) {
        auto setErr = [&](const std::string& m){ if (err) *err = m; };
        size_t i = 0;
        auto atEnd = [&]{ return i >= toks.size(); };
        auto peek = [&]() -> const cid::tok::Token& { return toks[i]; };
        auto next = [&]() -> const cid::tok::Token& { return toks[i++]; };
        auto expect = [&](cid::tok::TokenType t, const char* msg) -> bool {
            if (!atEnd() && peek().type == t) { next(); return true; }
            setErr(msg); return false;
        };

        while (!atEnd()) {
            const auto& t = peek();
            switch (t.type) {
                case cid::tok::PRINT: {
                    next();
                    if (atEnd()) { setErr("PRINT missing operand"); return false; }
                    const auto& lit = next();
                    if (lit.type == cid::tok::INT_LITERAL) {
                        if (lit.data.size() != sizeof(int)) { setErr("INT literal size mismatch"); return false; }
                        if (!expect(cid::tok::SEMICOLON, "missing ';' after PRINT int")) return false;
                    } else if (lit.type == cid::tok::STRING_LITERAL) {
                        if (!expect(cid::tok::SEMICOLON, "missing ';' after PRINT string")) return false;
                    } else {
                        setErr("PRINT expects literal"); return false;
                    }
                    break;
                }
                case cid::tok::RETURN: {
                    next();
                    if (atEnd()) { setErr("RETURN missing operand"); return false; }
                    const auto& lit = next();
                    if (lit.type != cid::tok::INT_LITERAL || lit.data.size() != sizeof(int)) {
                        setErr("RETURN expects int literal"); return false;
                    }
                    if (!expect(cid::tok::SEMICOLON, "missing ';' after RETURN")) return false;
                    break;
                }
                case cid::tok::SEMICOLON:
                    next(); // allow stray semicolons
                    break;
                default:
                    setErr("unsupported token at top-level"); return false;
            }
        }
        return true;
    }

    // Mantemos esta declaração para o pipeline futuro baseado em parser/otimizações.
    class CindraParserTree;
}
#endif //CINDRA_PARSER_H
