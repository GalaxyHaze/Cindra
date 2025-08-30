//
// Created by dioguabo-rei-delas on 8/13/25.
//
#ifndef CINDRA_HELPER_H
#define CINDRA_HELPER_H
#include <bitset>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <vector>

namespace cid::tok {
    enum TokenType : uint8_t;
}

namespace cid::help {
    constexpr inline bool isDigit(const char c) {
        return c >= '0' && c <= '9';
    }
    constexpr inline bool isAlpha(const char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }
    constexpr inline bool isHex(const char c) {
        return c >= '0' && c <= '9' || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }
    constexpr inline bool isSpace(const char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }
    constexpr inline bool isUnderscore(const char c) {
        return c == '_' || c == '-';
    }
    constexpr inline bool isOperator(const char c) {
        return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^' || c == '&';
    }
    constexpr inline auto convert_Digit(const char c) {
        if (c >= '0' && c <= '9') {
            return c - '0';
        }
        return -1;
    }
    template<typename T>
    inline auto to_byte(const T& object) {
        static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
        const auto size = sizeof(T);
        std::vector<std::byte> result(size);
        std::memcpy(result.data(), &object, size);
        return result;
    }
    inline std::vector<std::byte> to_byte(const std::string& object) {
        const auto size = object.size();
        std::vector<std::byte> result(size);
        std::memcpy(result.data(), object.data(), size);
        return result;
    }

    template<typename EnumType, size_t Size>
    class FunctionState {
        static_assert(std::is_enum_v<EnumType>, "EnumType must be an enum");

        using StatePtr = void*; // For label addresses (GCC/Clang extension)

        std::array<StatePtr, Size> states_{};
        std::bitset<Size> is_registered_{};

    public:
        FunctionState() = default;

        explicit FunctionState(const std::vector<EnumType>& enums,
                              const std::vector<StatePtr>& funcs) {
            if (enums.size() != funcs.size()) {
                throw std::invalid_argument("Mismatch between enums and functions size");
            }
            for (size_t i = 0; i < enums.size(); ++i) {
                Register(enums[i], funcs[i]);
            }
        }

        void Register(EnumType e, StatePtr label) {
            const auto idx = static_cast<size_t>(e);
            if (idx >= Size) {
                throw std::out_of_range("Enum value out of range");
            }
            if (label == nullptr) {
                throw std::invalid_argument("Label cannot be null");
            }
            if (is_registered_[idx]) {
                throw std::logic_error("State already registered");
            }
            states_[idx] = label;
            is_registered_[idx] = true;
        }

        StatePtr operator[](EnumType e) const noexcept {
            return states_[static_cast<size_t>(e)];
        }

        StatePtr operator[](EnumType e) noexcept {
            return states_[static_cast<size_t>(e)];
        }

        StatePtr operator[](size_t e) const noexcept {
            return states_[e];
        }

        StatePtr operator[](size_t e) noexcept {
            return states_[e];
        }

        StatePtr At(EnumType e) const {
            const auto idx = static_cast<size_t>(e);
            if (idx >= Size || !is_registered_[idx]) {
                HandleInvalidState();
            }
            return states_[idx];
        }

        bool Contains(EnumType e) const noexcept {
            const auto idx = static_cast<size_t>(e);
            return idx < Size && is_registered_[idx];
        }
        [[nodiscard]] bool empty() const noexcept {
            return is_registered_.none();
        }

    private:
        [[noreturn]] static void HandleInvalidState() {
            std::cerr << "Invalid state detected. Please contact support or check your plugins.\n";
            std::terminate();
        }
    };

    template<typename T>
    bool readSafe(const std::vector<uint8_t>& code, size_t& i, T& out) {
        if (i + sizeof(T) > code.size()) return false;
        std::memcpy(&out, &code[i], sizeof(T));
        i += sizeof(T);
        return true;
    }

    inline bool readStringSafe(const std::vector<uint8_t>& code, size_t& i, std::string& out) {
        if (i >= code.size()) return false;
        size_t len = code[i++];
        if (i + len > code.size()) return false;
        out.assign(reinterpret_cast<const char*>(&code[i]), len);
        i += len;
        return true;
    }

    inline void unsafeString(const uint8_t* src, size_t& i, uint8_t format) {

        auto length = *(++src);
        switch (format) {
            case 0:
                std::cout << reinterpret_cast<const char*>(++src);
                break;
            case 1:
                std::cout << *++src;
                break;
            default:
                break;
        }
        i += length;
    }


};


#endif //CINDRA_HELPER_H