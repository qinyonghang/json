#pragma once

#define OBJECT_IMPLEMENTATION

#include <cstdint>
#include <type_traits>

namespace qlib {

[[nodiscard]] constexpr static inline bool likely(bool ok) {
#ifdef __glibc_likely
    return __glibc_likely(ok);
#else
    return ok;
#endif
}

[[nodiscard]] constexpr static inline bool unlikely(bool ok) {
#ifdef __glibc_unlikely
    return __glibc_unlikely(ok);
#else
    return ok;
#endif
}

enum class memory_policy_t { copy, view };

using int8_t = std::int8_t;
using uint8_t = std::uint8_t;
using int16_t = std::int16_t;
using uint16_t = std::uint16_t;
using int32_t = std::int32_t;
using uint32_t = std::uint32_t;
using int64_t = std::int64_t;
using uint64_t = std::uint64_t;
using ssize_t = int64_t;
using size_t = uint64_t;
using float32_t = float;
using float64_t = double;

using bool_t = bool;
constexpr bool_t True = true;
constexpr bool_t False = false;

template <class T, class... Ts>
struct is_one_of final : std::disjunction<std::is_same<T, Ts>...> {};

template <class T, class... Ts>
inline constexpr bool_t is_one_of_v = is_one_of<T, Ts...>::value;

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
    using ssize_t = qlib::ssize_t;
    using size_t = qlib::size_t;
    using float32_t = qlib::float32_t;
    using float64_t = qlib::float64_t;

    using bool_t = qlib::bool_t;
    constexpr static inline bool_t True = qlib::True;
    constexpr static inline bool_t False = qlib::False;

protected:
    constexpr object() noexcept = default;
};

};  // namespace qlib
