//
// Created by dioguabo-rei-delas on 8/13/25.
//

#ifndef CINDRA_HELPER_H
#define CINDRA_HELPER_H
#include <cstring>
#include <type_traits>
#include <cassert>

namespace emb::help {
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

    constexpr inline auto convertInt(const char c) -> int8_t {
        return static_cast<int8_t>(c < '0' || c > '9' ? -1 : c - '0');
    }

    constexpr inline size_t to_digit(const std::string& src) {
        size_t result = 0;
        const size_t len = src.size();
        for (size_t i = len; i > 0; i--) {
            result += (10 * (len - 1)) * convertInt(src[i]) ;
        }
        return result;
    }

    template<typename T>
    inline auto to_byte( const T& object ) {
        assert( !std::is_trivial_v<T> && "Im sorry, but for now is UNSAFE copy raw things");
        const auto size = sizeof( T );
        std::vector<std::byte> result( size );
        std::memcpy( result.data(), &object, size );
        return result;
    }

    inline std::vector<std::byte> to_byte( const std::string& object ) {
        const auto size = object.size();
        std::vector<std::byte> result(size + 1);
        std::memcpy( result.data(), object.data(), size );
        result[size] = static_cast<std::byte>('\0');
        return result;
    }
}

//NOTE: Clion é foda! eu possivelmente demoraria uns minutinhos escrevedno tudo, mas ele auto - completou e tornou super eficiente!!!

#endif //CINDRA_HELPER_H