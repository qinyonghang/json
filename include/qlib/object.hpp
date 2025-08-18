#pragma once

#define OBJECT_IMPLEMENTATION

namespace qlib {

#define NODISCARD [[nodiscard]]
#define INLINE __attribute__((always_inline))

using int8_t = char;
using uint8_t = unsigned char;
using int16_t = short;
using uint16_t = unsigned short;
using int32_t = int;
using uint32_t = unsigned int;
using int64_t = long long;
using uint64_t = unsigned long long;
using float32_t = float;
using float64_t = double;

using bool_t = bool;
constexpr bool_t True = true;
constexpr bool_t False = false;

class object {
public:
    using int8_t = qlib::int8_t;
    using uint8_t = qlib::uint8_t;
    using int16_t = qlib::int16_t;
    using uint16_t = qlib::uint16_t;
    using int32_t = qlib::int32_t;
    using uint32_t = qlib::uint32_t;
    using int64_t = qlib::int64_t;
    using uint64_t = qlib::uint64_t;
    using float32_t = qlib::float32_t;
    using float64_t = qlib::float64_t;

    using bool_t = qlib::bool_t;
    constexpr static inline bool_t True{qlib::True};
    constexpr static inline bool_t False{qlib::False};

protected:
    INLINE constexpr object() noexcept = default;
};

NODISCARD INLINE constexpr auto likely(bool_t ok) {
    return __builtin_expect(ok, 1);
}

NODISCARD INLINE constexpr auto unlikely(bool_t ok) {
    return __builtin_expect(ok, 0);
}

class exception : public object {
public:
    virtual char const* what() const noexcept { return nullptr; }
};

template <bool_t, class T = void>
struct enable_if {};

template <class T>
struct enable_if<True, T> {
    using type = T;
};

template <bool_t Enable, class T = void>
using enable_if_t = typename enable_if<Enable, T>::type;

template <class T>
struct is_integral {
    constexpr static inline bool_t value{False};
};

template <>
struct is_integral<int8_t> {
    constexpr static inline bool_t value{True};
};

template <>
struct is_integral<uint8_t> {
    constexpr static inline bool_t value{True};
};

template <>
struct is_integral<int16_t> {
    constexpr static inline bool_t value{True};
};

template <>
struct is_integral<uint16_t> {
    constexpr static inline bool_t value{True};
};

template <>
struct is_integral<int32_t> {
    constexpr static inline bool_t value{True};
};

template <>
struct is_integral<uint32_t> {
    constexpr static inline bool_t value{True};
};

template <>
struct is_integral<int64_t> {
    constexpr static inline bool_t value{True};
};

template <>
struct is_integral<uint64_t> {
    constexpr static inline bool_t value{True};
};

template <class T>
constexpr static inline bool_t is_integral_v = is_integral<T>::value;

template <class T>
struct is_floating_point {
    constexpr static inline bool_t value{False};
};

template <>
struct is_floating_point<float32_t> {
    constexpr static inline bool_t value{True};
};

template <>
struct is_floating_point<float64_t> {
    constexpr static inline bool_t value{True};
};

template <class T>
constexpr static inline bool_t is_floating_point_v = is_floating_point<T>::value;

// template <class T, class... Ts>
// struct is_one_of : std::disjunction<std::is_same<T, Ts>...> {};

// template <class T, class... Ts>
// constexpr static inline bool_t is_one_of_v = is_one_of<T, Ts...>::value;

template <class, class>
struct is_same {
    constexpr static inline bool_t value{False};
};

template <class T>
struct is_same<T, T> {
    constexpr static inline bool_t value{True};
};

template <class T, class U>
constexpr static inline bool_t is_same_v = is_same<T, U>::value;

};  // namespace qlib
