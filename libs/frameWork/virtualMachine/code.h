//
// Created by dioguabo-rei-delas on 8/29/25.
//

#ifndef CINDRA_CODE_H
#define CINDRA_CODE_H
#include <string_view>
#include <type_traits>
#include <stdexcept>
#include "../tokens/tokenizer.h"
#include "../parser/parser.h"

namespace cid::code {

    // Append raw bytes from std::byte storage into uint8_t vector
    inline void insertByte(std::vector<uint8_t>& a, const std::vector<std::byte>& b) {
        a.insert(a.end(),
                 reinterpret_cast<const uint8_t*>(b.data()),
                 reinterpret_cast<const uint8_t*>(b.data()) + b.size());
    }

    // Small helpers to encode PODs and strings in our bytecode format
    template <typename T>
    inline void appendPOD(std::vector<uint8_t>& out, const T& value) {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        const auto* p = reinterpret_cast<const uint8_t*>(&value);
        out.insert(out.end(), p, p + sizeof(T));
    }

    inline void appendU8(std::vector<uint8_t>& out, uint8_t v) { out.push_back(v); }

    inline void appendLenString(std::vector<uint8_t>& out, std::string_view s) {
        // Limit to 255 for now (safe path will validate)
        if (s.size() > 255) throw std::runtime_error("string too long");
        appendU8(out, static_cast<uint8_t>(s.size()));
        const auto* p = reinterpret_cast<const uint8_t*>(s.data());
        out.insert(out.end(), p, p + s.size());
    }

class CODE {
    private:
        std::vector<uint8_t> code;
        explicit CODE(std::vector<uint8_t>&& codeVec) : code(std::move(codeVec)) {}

    public:
        [[nodiscard]] const std::vector<uint8_t>& getCode() const { return code; }

        // Only the bytecode generators can construct CODE instances
        friend CODE unsafePrototypeCode(const std::vector<tok::Token>&);
        friend CODE generateByteCode(const par::CindraParserTree&);
    };

    // Optional forward declaration if planner-based generation is added later
    CODE generateByteCode(const par::CindraParserTree& parser);

    // Bytecode format (minimal):
    // - PRINT: [PRINT opcode][type tag (INT_LITERAL|STRING_LITERAL)] [payload]
    //   - INT_LITERAL payload: 4 bytes (int)
    //   - STRING_LITERAL payload: 1-byte length, then bytes (no quotes)
    // - RETURN: [RETURN opcode][4-byte int]
    inline CODE unsafePrototypeCode(const std::vector<tok::Token>& src) {
        std::vector<uint8_t> code;
        code.reserve(src.size() * 6); // rough estimate to minimize reallocs

        auto encodeStringLiteral = [&](const tok::Token& t) {
            // token.lexeme includes quotes; remove them if present
            std::string_view sv{t.lexeme};
            if (sv.size() >= 2 && sv.front() == '"' && sv.back() == '"') {
                sv = sv.substr(1, sv.size() - 2);
            }
            appendLenString(code, sv);
        };

        for (size_t i = 0; i < src.size(); ++i) {
            const auto& t = src[i];
            switch (t.type) {
                case tok::PRINT: {
                    appendU8(code, static_cast<uint8_t>(tok::PRINT));
                    // Expect a literal next (int or string)
                    if (i + 1 >= src.size()) throw std::runtime_error("PRINT missing operand");
                    const auto& op = src[++i];
                    if (op.type == tok::INT_LITERAL) {
                        appendU8(code, static_cast<uint8_t>(tok::INT_LITERAL));
                        // data holds POD bytes for int
                        if (op.data.size() != sizeof(int))
                            throw std::runtime_error("INT literal encoding size mismatch");
                        insertByte(code, op.data);
                    } else if (op.type == tok::STRING_LITERAL) {
                        appendU8(code, static_cast<uint8_t>(tok::STRING_LITERAL));
                        encodeStringLiteral(op);
                    } else {
                        throw std::runtime_error("PRINT expects a literal operand");
                    }
                    break;
                }
                case tok::RETURN: {
                    appendU8(code, static_cast<uint8_t>(tok::RETURN));
                    if (i + 1 >= src.size()) throw std::runtime_error("RETURN missing operand");
                    const auto& op = src[++i];
                    if (op.type != tok::INT_LITERAL || op.data.size() != sizeof(int))
                        throw std::runtime_error("RETURN expects int literal");
                    insertByte(code, op.data);
                    break;
                }
                case tok::SEMICOLON:
                    // ignore
                    break;
                case tok::INT_LITERAL:
                case tok::STRING_LITERAL:
                    // Literals must be consumed by an instruction (PRINT/RETURN)
                    throw std::runtime_error("dangling literal without instruction");
                default:
                    throw std::runtime_error("unsupported token in code generation");
            }
        }

        return CODE(std::move(code));
    }

    // SAFE: portable, defensive checks, no computed gotos
    inline int safeRun(const CODE& src) {
        const auto& code = src.getCode();
        size_t i = 0;
        int returnValue = 0;

        while (i < code.size()) {
            const auto opcode = static_cast<tok::TokenType>(code[i++]);
            switch (opcode) {
                case tok::PRINT: {
                    if (i >= code.size()) throw std::runtime_error("PRINT missing type tag");
                    const auto typeTag = static_cast<tok::TokenType>(code[i++]);
                    if (typeTag == tok::INT_LITERAL) {
                        int value{};
                        if (!cid::help::readSafe(code, i, value))
                            throw std::runtime_error("truncated int literal in PRINT");
                        std::cout << value;
                    } else if (typeTag == tok::STRING_LITERAL) {
                        std::string str;
                        if (!cid::help::readStringSafe(code, i, str))
                            throw std::runtime_error("truncated string literal in PRINT");
                        std::cout << str;
                    } else {
                        throw std::runtime_error("invalid type tag in PRINT");
                    }
                    break;
                }
                case tok::RETURN: {
                    int value{};
                    if (!cid::help::readSafe(code, i, value))
                        throw std::runtime_error("truncated return value");
                    returnValue = value;
                    // Program terminates on RETURN
                    return returnValue;
                }
                default:
                    throw std::runtime_error("invalid opcode encountered");
            }
        }
        return returnValue;
    }

    // UNSAFE: fast path, assumes well-formed code, uses computed gotos and minimal checks
    inline int unsafeRun(const CODE& src) {
        const auto& code = src.getCode();
        if (code.empty()) return 0;

        size_t i = 0;
        cid::help::FunctionState<tok::TokenType, 256> dTable;
        dTable.Register(tok::PRINT, &&PRINT);
        dTable.Register(tok::RETURN, &&RETURN);

        DISPATCH:
        {
            const uint8_t op = code[i++];
            goto *dTable[op ? op : static_cast<uint8_t>(tok::INVALID)];
        }

        PRINT: {
            const uint8_t type = code[i++];
            if (type == static_cast<uint8_t>(tok::INT_LITERAL)) {
                const int v = *reinterpret_cast<const int*>(&code[i]);
                i += sizeof(int);
                std::cout << v;
                goto DISPATCH;
            } else if (type == static_cast<uint8_t>(tok::STRING_LITERAL)) {
                const uint8_t len = code[i++];
                std::cout.write(reinterpret_cast<const char*>(&code[i]), len);
                i += len;
                goto DISPATCH;
            } else {
                return 0; // invalid
            }
        }

        RETURN: {
            const int v = *reinterpret_cast<const int*>(&code[i]);
            return v;
        }
    }

}
#endif //CINDRA_CODE_H
