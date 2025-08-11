#pragma once

#define STRING_IMPLEMENTATION

#include <charconv>
#include <memory>

#include "qlib/object.h"

namespace qlib {

namespace string {

template <class Char>
[[nodiscard]] constexpr size_t strlen(Char const* str) noexcept {
    size_t size{0u};
    while (str[size] != '\0') {
        ++size;
    }
    return size;
}

template <class Char>
[[nodiscard]] constexpr bool_t in(Char c, Char const* str) noexcept {
    bool_t ok{False};
    while (*str != '\0') {
        if (unlikely(c == *str)) {
            ok = True;
            break;
        }
        ++str;
    }
    return ok;
}

template <class Char, memory_policy_t Policy = memory_policy_t::copy>
class value;

template <class Char>
struct char_traits final : public object {
    using value_type = Char;
    using pointer = value_type*;
    using const_pointer = value_type const*;
    using reference = value_type&;
    using const_reference = value_type const&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using size_type = size_t;
    static constexpr size_type npos = size_type(-1);
};

class bad_to final : public std::exception {
public:
    char const* what() const noexcept override { return "bad to"; }
};

class bad_from final : public std::exception {
public:
    char const* what() const noexcept override { return "bad from"; }
};

template <class Char>
class value<Char, memory_policy_t::view> final : public object {
public:
    using self = value<Char, memory_policy_t::view>;
    using traits_type = char_traits<Char>;
    using value_type = typename traits_type::value_type;
    using const_pointer = typename traits_type::const_pointer;
    using const_reference = typename traits_type::const_reference;
    using const_iterator = typename traits_type::const_iterator;
    using size_type = typename traits_type::size_type;
    static constexpr size_type npos = traits_type::npos;

protected:
    const_pointer _impl{nullptr};
    size_type _size{0u};

public:
    constexpr value() noexcept = default;

    template <class Iter1, class Iter2>
    explicit constexpr value(Iter1 begin, Iter2 end) noexcept
            : _impl(begin), _size(std::distance(begin, end)) {}

    constexpr value(const_pointer o) noexcept : _impl(o), _size(strlen(o)) {}

    constexpr value(self const& o) : _impl(o._impl), _size(o._size) {}

    constexpr self& operator=(const_pointer o) {
        _impl = o;
        _size = strlen(o);
        return *this;
    }

    constexpr self& operator=(self const& o) {
        if (unlikely(this != &o)) {
            _impl = o._impl;
            _size = o._size;
        }
        return *this;
    }

    [[nodiscard]] constexpr const_pointer data() const noexcept { return _impl; }

    [[nodiscard]] constexpr size_type size() const noexcept { return _size; }

    [[nodiscard]] constexpr self substr(size_type pos, size_type n = npos) const noexcept {
        if (unlikely(pos > size())) {
            return self();
        }
        size_type len = (n == npos || pos + n > size()) ? size() - pos : n;
        return self(data() + pos, data() + pos + len);
    }

    [[nodiscard]] constexpr bool_t starts_with(const_pointer o) const noexcept {
        return starts_with(self(o));
    }

    [[nodiscard]] constexpr bool_t starts_with(self const& o) const noexcept {
        return substr(0u, o.size()) == o;
    }

    [[nodiscard]] constexpr bool_t ends_with(const_pointer o) const noexcept {
        return ends_with(self(o));
    }

    [[nodiscard]] constexpr bool_t ends_with(self const& o) const noexcept {
        return size() >= o.size() && substr(size() - o.size(), o.size()) == o;
    }

    [[nodiscard]] constexpr bool_t operator==(const_pointer o) const noexcept {
        return *this == self(o);
    }

    [[nodiscard]] constexpr bool_t operator==(self const& o) const noexcept {
        return size() == o.size() && std::equal(begin(), end(), o.begin());
    }

    template <class T>
    [[nodiscard]] constexpr bool_t operator!=(T o) const noexcept {
        return !(*this == o);
    }

    [[nodiscard]] constexpr auto operator[](size_type pos) const noexcept {
        return *(data() + pos);
    }

    [[nodiscard]] constexpr auto empty() const noexcept { return size() == 0u; }

    [[nodiscard]] constexpr auto begin() const noexcept { return data(); }

    [[nodiscard]] constexpr auto end() const noexcept { return data() + size(); }

    [[nodiscard]] constexpr auto front() const noexcept { return *(data()); }

    [[nodiscard]] constexpr auto back() const noexcept { return *(data() + size() - 1); }

#ifdef _GLIBCXX_STRING_VIEW
    [[nodiscard]] operator std::basic_string_view<value_type>() const noexcept {
        return std::basic_string_view<value_type>(data(), size());
    }
#endif

    [[nodiscard]] explicit operator bool_t() const noexcept { return !empty(); }

    template <class T,
              class Enable = std::enable_if_t<is_one_of_v<T,
                                                          bool_t,
                                                          int8_t,
                                                          int16_t,
                                                          int32_t,
                                                          int64_t,
                                                          uint8_t,
                                                          uint16_t,
                                                          uint32_t,
                                                          uint64_t,
                                                          float32_t,
                                                          float64_t>>>
    [[nodiscard]] constexpr T to() const {
        if constexpr (std::is_same_v<T, bool_t>) {
            if (*this == "true" || *this == "True") {
                return True;
            } else if (*this == "false" || *this == "False") {
                return False;
            } else {
                throw bad_to();
            }
        } else {
            T result;
            auto [ptr, ec] = std::from_chars(begin(), end(), result);
            if (unlikely(ec != std::errc{})) {
                throw bad_to();
            }
            return result;
        }
    }
};

template <class Char>
class value<Char, memory_policy_t::copy> final : public object {
public:
    using self = value<Char, memory_policy_t::copy>;
    using string_view_type = value<Char, memory_policy_t::view>;
    using traits_type = char_traits<Char>;
    using value_type = typename traits_type::value_type;
    using pointer = typename traits_type::pointer;
    using const_pointer = typename traits_type::const_pointer;
    using reference = typename traits_type::reference;
    using const_reference = typename traits_type::const_reference;
    using iterator = typename traits_type::iterator;
    using const_iterator = typename traits_type::const_iterator;
    using size_type = typename traits_type::size_type;
    static constexpr size_type npos = traits_type::npos;

protected:
    template <class T>
    using uptr = std::unique_ptr<T>;

    uptr<Char[]> _impl{nullptr};
    size_type _size{0u};
    size_type _capacity{0u};

    template <class T>
    [[nodiscard]] static inline self _from(T value, size_type capacity) {
        self result{capacity};
        auto [ptr, ec] = std::to_chars(result.data(), result.data() + capacity, value);
        if (unlikely(ec != std::errc{})) {
            throw bad_from{};
        }
        result._size = std::distance(result.data(), ptr);
        return result;
    }

    template <class Iter1, class Iter2>
    inline void _assign(Iter1 begin, Iter2 end) {
        const size_type size = std::distance(begin, end);
        if (likely(size > 0)) {
            if (likely(size > capacity())) {
                _impl = uptr<Char[]>(new Char[size + 1u]);
                _capacity = size;
            }
            std::copy(begin, end, _impl.get());
            _impl[size] = '\0';
            _size = size;
        }
    }

public:
    constexpr value() noexcept = default;

    explicit value(size_type capacity) : _capacity(capacity) {
        if (likely(capacity > 0)) {
            _impl = uptr<Char[]>(new Char[capacity + 1u]);
        }
    }

    template <class Iter1, class Iter2>
    explicit value(Iter1 begin, Iter2 end) {
        _assign(begin, end);
    }

    value(const_pointer str) : value(str, str + strlen(str)) {}

    value(string_view_type o) : value(o.begin(), o.end()) {}

    value(self const& o) : value(o.begin(), o.end()) {}

    value(self&& o) noexcept : _impl(std::move(o._impl)), _size(o._size), _capacity(o._capacity) {
        o._size = 0u;
        o._capacity = 0u;
    }

#ifdef _BASIC_STRING_H
    explicit value(std::basic_string_view<Char> str) : value(str.data(), str.data() + str.size()) {}
#endif

    self& operator=(const_pointer o) { return *this = string_view_type(o); }

    self& operator=(string_view_type o) {
        _assign(o.begin(), o.end());
        return *this;
    }

    self& operator=(self const& o) {
        if (unlikely(this != &o)) {
            _assign(o.begin(), o.end());
        }
        return *this;
    }

    self& operator=(self&& o) noexcept {
        if (unlikely(this != &o)) {
            _impl = std::move(o._impl);
            _size = o._size;
            _capacity = o._capacity;
            o._impl = nullptr;
            o._size = 0u;
            o._capacity = 0u;
        }
        return *this;
    }

    void reserve(size_type capacity) {
        if (likely(capacity > this->capacity())) {
            auto impl = uptr<Char[]>(new Char[capacity + 1u]);
            std::copy(begin(), end(), impl.get());
            _impl = std::move(impl);
            _impl[_size] = '\0';
            _capacity = capacity;
        }
    }

    [[nodiscard]] constexpr pointer data() noexcept { return _impl.get(); }

    [[nodiscard]] constexpr const_pointer data() const noexcept { return _impl.get(); }

    [[nodiscard]] constexpr size_type size() const noexcept { return _size; }

    [[nodiscard]] constexpr size_type capacity() const noexcept { return _capacity; }

    [[nodiscard]] constexpr bool_t empty() const noexcept { return size() == 0u; }

    [[nodiscard]] constexpr reference operator[](size_type pos) noexcept { return data()[pos]; }

    [[nodiscard]] constexpr const_reference operator[](size_type pos) const noexcept {
        return data()[pos];
    }

    [[nodiscard]] constexpr iterator begin() noexcept { return data(); }

    [[nodiscard]] constexpr const_iterator begin() const noexcept { return data(); }

    [[nodiscard]] constexpr iterator end() noexcept { return data() + size(); }

    [[nodiscard]] constexpr const_iterator end() const noexcept { return data() + size(); }

    [[nodiscard]] constexpr reference front() noexcept { return *data(); }

    [[nodiscard]] constexpr const_reference front() const noexcept { return *data(); }

    [[nodiscard]] constexpr reference back() noexcept { return *(data() + size() - 1); }

    [[nodiscard]] constexpr const_reference back() const noexcept { return *(data() + size() - 1); }

    [[nodiscard]] constexpr const_pointer c_str() const noexcept { return data(); }

    [[nodiscard]] constexpr bool_t starts_with(self const& o) const noexcept {
        return static_cast<string_view_type>(*this).starts_with(o);
    }

    [[nodiscard]] constexpr bool_t ends_with(self const& o) const noexcept {
        return static_cast<string_view_type>(*this).ends_with(o);
    }

    [[nodiscard]] constexpr bool_t operator==(string_view_type o) const noexcept {
        return size() == o.size() && std::equal(begin(), end(), o.begin());
    }

    template <class T>
    [[nodiscard]] constexpr bool_t operator!=(T o) const noexcept {
        return !(*this == o);
    }

    template <class Iter1, class Iter2>
    void emplace(Iter1 __begin, Iter2 __end) {
        const size_type __size = std::distance(__begin, __end);
        if (likely(__size > 0u)) {
            const size_type __new_size = size() + __size;
            if (unlikely(__new_size > capacity())) {
                size_type const __new_capacity = __new_size * 2;
                auto __impl = uptr<Char[]>(new Char[__new_capacity + 1u]);
                std::copy(begin(), end(), __impl.get());
                _impl = std::move(__impl);
                _capacity = __new_capacity;
            }
            std::copy(__begin, __end, end());
            _impl[__new_size] = '\0';
            _size = __new_size;
        }
    }

    self& operator<<(const_pointer o) {
        emplace(o, o + strlen(o));
        return *this;
    }

    self& operator<<(string_view_type o) {
        emplace(o.begin(), o.end());
        return *this;
    }

    self& operator<<(self const& o) {
        emplace(o.begin(), o.end());
        return *this;
    }

    [[nodiscard]] operator string_view_type() const noexcept {
        return string_view_type(begin(), end());
    }

#ifdef _BASIC_STRING_H
    [[nodiscard]] operator std::basic_string<Char>() const noexcept {
        return std::basic_string<Char>(data());
    }
#endif

    [[nodiscard]] static self from(bool_t value) { return value ? "True" : "False"; }
    [[nodiscard]] static self from(int8_t value) { return _from(value, 3u); }
    [[nodiscard]] static self from(int16_t value) { return _from(value, 5u); }
    [[nodiscard]] static self from(int32_t value) { return _from(value, 10u); }
    [[nodiscard]] static self from(int64_t value) { return _from(value, 20u); }
    [[nodiscard]] static self from(uint8_t value) { return _from(value, 3u); }
    [[nodiscard]] static self from(uint16_t value) { return _from(value, 5u); }
    [[nodiscard]] static self from(uint32_t value) { return _from(value, 10u); }
    [[nodiscard]] static self from(uint64_t value) { return _from(value, 20u); }
    [[nodiscard]] static self from(float32_t value) { return _from(value, 58u); }
    [[nodiscard]] static self from(float64_t value) { return _from(value, 328u); }

    template <class T>
    [[nodiscard]] T to() const {
        return static_cast<string_view_type>(*this).template to<T>();
    }
};

#ifdef _GLIBCXX_OSTREAM
template <class Char, memory_policy_t Policy>
std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& out, value<Char, Policy> const& s) {
    std::__ostream_insert(out, s.data(), s.size());
    return out;
}
#endif

};  // namespace string

using string_view_t = string::value<char, memory_policy_t::view>;
using string_t = string::value<char, memory_policy_t::copy>;

};  // namespace qlib
