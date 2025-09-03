#ifndef LAZYANY_LIBRARY_H
#define LAZYANY_LIBRARY_H
#include <cstddef>
#include <type_traits>
#include <atomic>
#include <cassert>
#include <cstring>
#include <new>
#include <array>
#include <string_view>
//#include <expected>
#include "dynamicBitSet.h"

namespace lazy {
    using size_t = std::size_t;
    constexpr size_t SBO = 8;
    constexpr size_t types = 32;

    namespace detail {
        enum  utilityFunc {
            //CONSTRUCTOR, //I mean... That dont make to much sense?
            DESTRUCTOR,
            MOVE, // Move constructor
            COPY, // Move copy
            ASSIGN_CPY, //const T&
            ASSIGN_MOVE, //T&&
            PLUS,
            MINUS,
            MULTIPLY,
            DIVIDE,
            PLUS_ASSIGN,
            MINUS_ASSIGN,
            MULTIPLY_ASSIGN,
            DIVIDE_ASSIGN,
            EQUAL,
            NOT_EQUAL,
            LESS,
            LESS_EQUAL,
            GREATER,
            GREATER_EQUAL,
            CAST,
            ID,
            RUNTIME_TYPE,
            S_B_O,
            SIZEOF,
            REGISTER
        };
    }

    class counter {
        inline static std::atomic<uint16_t> Gid = 0;
        template<class T>
        friend struct IdInfo;
    public: counter() = delete;
    };
    template<class T>
    struct IdInfo{
        static auto getID() {
            const static auto id = counter::Gid.fetch_add(1, std::memory_order_relaxed);
            assert(id < types && "Maximum type capacity reached!");
            return id;
        }
    };

    //forward declaration
    class any;
    template<class T>
    T cast_to(const any&);

    class Operator {
        using fn_1 = void(*)(void*);
        using fn_2 = void(*)(void*,void*);
        //OK, needs a lot of explanation, but this function CAN NOT be used outside the Interface class
        //because it has a really defined usage, fn_1 refer to ONLY destructor
        //and all other 'functions' are operators that requires the original object, and an external object
        //do not make sense to use variant here, that struct ONLY handle this and nothing more, and shoudln't do anyhthing else
        struct Func {
            union {
                fn_1 oneSrc;
                fn_2 twoSrc;
            };
            bool isOne = false;
            explicit Func(fn_1 fn) : oneSrc(fn), isOne(true) {}
            explicit Func(fn_2 fn) : twoSrc(fn), isOne(false) {}
            Func(): oneSrc(nullptr){};
            //explicit Func(const Func& o): oneSrc(o.oneSrc), isOne(o.isOne) {}
            void operator()(void* src1, void* src2 = nullptr) const{
                if (!oneSrc && !twoSrc) {
                    throw std::bad_function_call();
                }
                if (isOne) {
                    oneSrc(src1);
                }
                else {
                    if (!src1 || !src2)
                        throw std::invalid_argument("Invalid argument, since one o the two sources was nullptr");
                    twoSrc(src1, src2);
                }
            }

        };

        dynamicBitset Mask{types};
        Func lambdas[types];
    public:
        void Register(detail::utilityFunc state, fn_2 expr) {
            auto i = static_cast<size_t>(state);
            assert(i < types && "State exceeds type limit!");
            lambdas[i] = Func(expr);
        }
    };

    using Fn_expr = void(*)(void*, detail::utilityFunc, void*, uint16_t);
    template<class T>
    class utilityLambda {
        static  void dispatch(void* src, detail::utilityFunc state, void* dest, uint16_t other = 0) {
            //TODO utility func
            static constexpr bool isSBO = sizeof(T) <= SBO;
            static const std::type_info* meta = &typeid(T);
            static std::array<Operator, 16> operators;
            using namespace detail;
            switch (state) {
                case DESTRUCTOR:
                    if (!src) return;
                    if constexpr (!std::is_trivially_destructible_v<T>) {
                        static_cast<T*>(src)->~T();
                    }
                    if (!isSBO) std::free(src);
                    return;
                case S_B_O:
                    *static_cast<bool*>(dest) = isSBO;
                    return;
                case SIZEOF:
                    *static_cast<size_t*>(dest) = sizeof(T);
                    return;
                case ID:
                    *static_cast<uint16_t*>(dest) = IdInfo<T>::getID();
                    return;
                case RUNTIME_TYPE:
                    *static_cast<const std::type_info**>(dest) = meta;
                    return;
                case COPY:
                    if constexpr (std::is_copy_constructible_v<T>) new (dest) T(*static_cast<T*>(src));
                    else throw std::runtime_error("Type not copyable");
                    return;
                case MOVE:
                    if constexpr (std::is_move_constructible_v<T>) {
                        new (dest) T(std::move(*static_cast<T*>(src)));
                        static_cast<T*>(src)->~T();
                    } else throw std::runtime_error("Type not movable");
                    return;
                case REGISTER:
                    *static_cast<Operator*>(dest) = operators[other];
                    return;
                default:
                    throw std::runtime_error("Unsupported operation");
            }
        }
        public: utilityLambda() = delete;
        friend class any;
    };

    class any{
        union {
            alignas(std::max_align_t) char buffer[SBO];
            void* ptr = nullptr;
        };
        Fn_expr metaData = nullptr;

        public:
        any() = default;
        template<class T>
        explicit any(const T& o) {
            metaData = utilityLambda<T>::dispatch;
            if (!metaData)
                throw std::bad_alloc();
            constexpr bool isSBO = sizeof(T) <= SBO;
            void* place = isSBO? static_cast<void*>(buffer) : ptr = malloc(sizeof(T));
            place? new (place) T(o) : throw std::bad_alloc();
        }
        template<class T, typename... Args>
        explicit any(Args&&... args) {
            metaData = utilityLambda<T>::dispatch;
            if (!metaData)
                throw std::bad_alloc();
            constexpr bool isSBO = sizeof(T) <= SBO;
            void* place = isSBO? static_cast<void*>(buffer) : ptr = malloc(sizeof(T));
            place? new (place) T(std::forward<Args>(args)...) : throw std::bad_alloc();

        }
        any(const any& o) {
            size_t size;
            if (!o.metaData)
                throw std::bad_function_call();
            metaData = o.metaData;
            metaData(nullptr, detail::SIZEOF, &size, 0);
            void* place = size < SBO? static_cast<void*>(buffer) : ptr = malloc(size);
            place? metaData(o.get(), detail::COPY,place, 0) : throw std::bad_alloc();
        }
        any(any&& o)  noexcept {
            if (o.fallToAllocator()) {
                ptr = o.ptr;
                o.ptr = nullptr;
            } else {
                std::memcpy(buffer, o.buffer, SBO);
            }
            metaData = o.metaData;
            o.metaData = nullptr;

        }

        [[nodiscard]] bool fallToAllocator() const {
            if (!metaData)
                throw std::bad_function_call();
            bool r;
            metaData(ptr, detail::S_B_O, &r, 0);
            return !r;
        }

        [[nodiscard]] bool empty() const noexcept {
            return !metaData;
        }

        [[nodiscard]] void* get() const noexcept {
            //i just dont know what happen, but compiller transf the array into a const char* so it's need to const cast
            return fallToAllocator()? ptr : const_cast<void*>(reinterpret_cast<const void*>(buffer));
        }

        [[nodiscard]] uint16_t getType() const {
            if (!metaData)
                throw std::bad_function_call();
            uint16_t result;
            metaData(ptr, detail::ID, &result, 0);
            return result;
        }

        [[nodiscard]] const std::type_info& typeID() const {
            if (!metaData)
                throw std::bad_function_call();
            std::type_info* i;
            metaData(ptr, detail::RUNTIME_TYPE, &i, 0);
            return *i;
        }

        [[nodiscard]] auto name() const {
            if (!metaData)
                throw std::bad_function_call();
            std::type_info* i;
            metaData(ptr, detail::RUNTIME_TYPE, &i, 0);
            return std::string_view(i->name());
        }

        template<class T>
        [[nodiscard]] bool is_same() const{
            if (!metaData)
                throw std::bad_function_call();
            return getType() == IdInfo<T>::getID();
        }

        template<class T>
        [[nodiscard]] bool is_not_same() const{
            return getType() != IdInfo<T>::getID();
        }

        [[nodiscard]] bool is_same(const any& o) const{
            if (!metaData)
                throw std::bad_function_call();
            return getType() == o.getType();
        }

        [[nodiscard]] bool is_not_same(const any& o) const {
            return getType() != o.getType();
        }

        template<class T>
        operator T() const{
            if (!metaData)
                throw std::bad_function_call();
            return cast_to<T>(*this);
        }

        ~any() {
            if (metaData) {
                metaData(get(), detail::DESTRUCTOR, nullptr, 0);
                if (fallToAllocator())
                    ptr = nullptr;
                metaData = nullptr;
            }

        }

    };

    template<class T>
    T cast_to(const any& obj) {
        if (obj.empty())
            throw std::bad_cast();
        if (obj.getType() == IdInfo<T>::getID() )
            return *static_cast<T*>(obj.get());
        throw std::bad_cast();
    }
    template<class T>
    T& cast_to_ref(const any& obj) {
        if (obj.empty()) {
            throw std::bad_cast();
        }
        if (obj.getType() == IdInfo<T>::getID()) {
            return *static_cast<T*>(obj.get());
        }
        throw std::bad_cast();
    }
    //TODO: it's needed to polish the dispatch function inside utilityFunc class, inseert the register functions, and custom deleters and allocators, but technically it's working, i still do not test, but looks ok

}
#endif // LAZYANY_LIBRARY_H