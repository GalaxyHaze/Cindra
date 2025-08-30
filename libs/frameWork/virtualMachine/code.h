//
// Created by dioguabo-rei-delas on 8/29/25.
//

#ifndef CINDRA_CODE_H
#define CINDRA_CODE_H
#include "../tokens/tokenizer.h"
#include "../parser/parser.h"

namespace cid::code {


    inline void insertByte(std::vector<uint8_t>& a, const std::vector<std::byte>& b) {
        a.insert(a.end(),
                 reinterpret_cast<const uint8_t*>(b.data()),
                 reinterpret_cast<const uint8_t*>(b.data()) + b.size());
    }

    class CODE {
    private:
        std::vector<uint8_t> code;

    public:
        explicit CODE(std::vector<uint8_t>&& codeVec) : code(std::move(codeVec)) {}

        // Provide access to the code
        [[nodiscard]] const std::vector<uint8_t>& getCode() const { return code; }

        // Allow the code to be modified if needed
        std::vector<uint8_t>& getMutableCode() { return code; }

        // Friend declarations for functions that need access to private members
        friend CODE generateCode(const std::vector<tok::Token>&);
        friend int run(const CODE&);
    };

    // Forward declaration for the unsafe bytecode generation
    CODE generateByteCode(const par::CindraParserTree& parser);

    inline CODE generateByteCode(const std::vector<tok::Token>& src) {
        // Calculate total size needed
        size_t totalSize = 0;
        for (const auto& token : src) {
            if (token.type == tok::INT_LITERAL || token.type == tok::STRING_LITERAL) {
                totalSize += token.data.size();
            } else if (token.type != tok::SEMICOLON){
                totalSize += 1; // One byte for the token type
            }
        }

        std::vector<uint8_t> code;
        code.reserve(totalSize);

        for (const auto& token : src) {
            if (token.type == tok::INT_LITERAL || token.type == tok::STRING_LITERAL) {
                insertByte(code, token.data);
            } else if (token.type != tok::SEMICOLON) {
                code.push_back(token.type);
            }
        }

        // Check if we used the expected amount of space
        if (code.size() != totalSize) {
            throw std::runtime_error("Code size doesn't match expected size");
        }

        return CODE(std::move(code));
    }

    // The function returns 'int' as the program's exit code
    inline int safeRun(const CODE& src) {
        const auto& code = src.getCode();
        if (code.empty()) {
            return 0; // Empty program
        }

        size_t i = 0;
        int returnValue = 0; // Default return value

        // Setup dispatch table for token types
        // Initialize all entries to END
        const void* dispatchTable[256] = { &&END };

        // Set up specific handlers for known token types
        dispatchTable[static_cast<uint8_t>(tok::TokenType::PRINT)] = &&PRINT;
        dispatchTable[static_cast<uint8_t>(tok::TokenType::RETURN)] = &&RETURN;
        // Add more handlers as needed for your token types

        // Main dispatch loop
        DISPATCH:
        if (i >= code.size()) goto END;
        goto *dispatchTable[code[i++]];

    PRINT:
        // Implementation for PRINT instruction
        {
            // Check if we have enough bytes for a token type
            if (i >= code.size()) goto END;

            // Get the next token type to determine what to print

            switch (code[i++]) {
                case static_cast<uint8_t>(tok::TokenType::INT_LITERAL): {
                    // Check if we have enough bytes for an integer
                    if (i + sizeof(int) > code.size()) goto END;

                    int value;
                    std::memcpy(&value, &code[i], sizeof(int));
                    i += sizeof(int);
                    std::cout << value;
                    break;
                }
                case static_cast<uint8_t>(tok::TokenType::STRING_LITERAL): {
                    // Check if we have at least one byte for the length
                    if (i >= code.size()) goto END;

                    // Get the string length
                    size_t strLen = code[i++];

                    // Check if we have enough bytes for the string
                    if (i + strLen > code.size()) goto END;

                    std::string str(reinterpret_cast<const char*>(&code[i]), strLen);
                    i += strLen;
                    std::cout << str;
                    break;
                }
                default:
                    // Unknown token type for PRINT
                    goto END;
            }

            // Continue to next instruction
            goto DISPATCH;
        }

    RETURN:
        // Implementation for RETURN instruction
        {
            // Check if we have enough bytes for a return value
            if (i + sizeof(int) <= code.size()) {
                std::memcpy(&returnValue, &code[i], sizeof(int));
            }
            goto END;
        }

    END:
        return returnValue;
    }
    inline int unsafeRun(const CODE& src) {
        const auto& code = src.getCode();
        if (code.empty()) {
            return 0;
        }
        size_t i = 0;
        help::FunctionState<tok::TokenType, 512> dTable;
        dTable.Register(tok::RETURN, &&RETURN);
        DISPATCH:
        goto *dTable[code[i++]];

        //PRINT:
        //goto DISPATCH;
        RETURN:
        return code[i];
        END:
        return 0;
    }

}
#endif //CINDRA_CODE_H