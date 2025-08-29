//
// Created by dioguabo-rei-delas on 8/13/25.
//
#ifndef CINDRA_HELPER_H
#define CINDRA_HELPER_H
#include <bitset>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <vector>

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
    /*template<typename E, typename Vt, size_t Si>
    class denseStatemachine {
        std::array<Vt, Si> states;
        std::array<bool, Si>isStateSigned;
        std::array<bool, Si>isIndexSigned;
        std::array<Vt, Si> index[Si];
        void init() {
            for ( size_t i = 0; i < Si; i++ ) {
                isStateSigned[i] = false;
                isIndexSigned[i] = false;

            }
        }
        public:
        denseStatemachine() {
            init();
        }
        explicit denseStatemachine( const std::vector<E>& states ) {
            init();
            for ( const auto& i : states ) {
                Register(i);
            }
        }
        void Register(E e){
            if (isStateSigned[static_cast<Vt>(e)])
                return;
            for ( size_t i = 0; i < Si; i++ ) {
                if (!isIndexSigned[i]) {
                    isStateSigned[i] = true;
                    isIndexSigned[i] = true;
                    index[i] = static_cast<Vt>(e);
                    return;
                }
            }
            throw std::runtime_error("denseVector index out of range");
        }
        auto operator[](const Vt dIndex) {
            return static_cast<E>(index[dIndex]);
        }
        auto at(const Vt dIndex) {
            if (!isIndexSigned[dIndex]) {
                throw std::runtime_error("denseVector index out of range");
            }
            return static_cast<E>(index[dIndex]);
        }

    };*/
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



};


#endif //CINDRA_HELPER_H