#pragma once

#define STRING_IMPLEMENTATION

#include <charconv>

#include "qlib/memory.h"
#include "qlib/object.h"

namespace qlib {

namespace string {

template <class Char>
NODISCARD INLINE constexpr uint64_t strlen(Char const* str) noexcept {
    uint64_t size{0u};
    while (str[size] != '\0') {
        ++size;
    }
    return size;
}

template <class Char>
NODISCARD INLINE constexpr bool_t in(Char c, Char const* str) noexcept {
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

class bad_to final : public exception {
public:
    char const* what() const noexcept override { return "bad to"; }
};

class bad_from final : public exception {
public:
    char const* what() const noexcept override { return "bad from"; }
};

class out_of_range final : public exception {
public:
    char const* what() const noexcept override { return "out of range"; }
};

template <class Char>
struct char_traits final : public object {
    using value_type = Char;
    using pointer = value_type*;
    using const_pointer = value_type const*;
    using reference = value_type&;
    using const_reference = value_type const&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using size_type = uint32_t;
    static constexpr size_type npos = size_type(-1);
};

template <class Char>
class view final : public object {
public:
    using self = view;
    using char_type = Char;
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
    INLINE constexpr view() noexcept = default;

    template <class Iter1, class Iter2>
    INLINE explicit constexpr view(Iter1 begin, Iter2 end) noexcept
            : _impl(begin), _size(std::distance(begin, end)) {}

    INLINE constexpr view(const_pointer o) noexcept : _impl(o), _size(strlen(o)) {}

    INLINE constexpr view(self const& o) noexcept : _impl(o._impl), _size(o._size) {}

    INLINE constexpr self& operator=(const_pointer o) noexcept {
        _impl = o;
        _size = strlen(o);
        return *this;
    }

    INLINE constexpr self& operator=(self const& o) noexcept {
        if (unlikely(this != &o)) {
            _impl = o._impl;
            _size = o._size;
        }
        return *this;
    }

    NODISCARD INLINE constexpr const_pointer data() const noexcept { return _impl; }

    NODISCARD INLINE constexpr size_type size() const noexcept { return _size; }

    NODISCARD INLINE constexpr self substr(size_type pos, size_type n = npos) const noexcept {
        if (unlikely(pos > size())) {
            return self();
        }
        size_type len = (n == npos || pos + n > size()) ? size() - pos : n;
        return self(data() + pos, data() + pos + len);
    }

    NODISCARD INLINE constexpr bool_t starts_with(const_pointer o) const noexcept {
        return starts_with(self(o));
    }

    NODISCARD INLINE constexpr bool_t starts_with(self const& o) const noexcept {
        return substr(0u, o.size()) == o;
    }

    NODISCARD INLINE constexpr bool_t ends_with(const_pointer o) const noexcept {
        return ends_with(self(o));
    }

    NODISCARD INLINE constexpr bool_t ends_with(self const& o) const noexcept {
        return size() >= o.size() && substr(size() - o.size(), o.size()) == o;
    }

    NODISCARD INLINE constexpr bool_t operator==(const_pointer o) const noexcept {
        return *this == self(o);
    }

    NODISCARD INLINE constexpr bool_t operator==(self const& o) const noexcept {
        return size() == o.size() && std::equal(begin(), end(), o.begin());
    }

    template <class T>
    NODISCARD INLINE constexpr bool_t operator!=(T o) const noexcept {
        return !(*this == o);
    }

    NODISCARD INLINE constexpr auto operator[](size_type pos) const noexcept {
        return *(data() + pos);
    }

    NODISCARD INLINE constexpr auto empty() const noexcept { return size() == 0u; }

    NODISCARD INLINE constexpr auto begin() const noexcept { return data(); }

    NODISCARD INLINE constexpr auto end() const noexcept { return data() + size(); }

    NODISCARD INLINE constexpr auto front() const noexcept { return *(data()); }

    NODISCARD INLINE constexpr auto back() const noexcept { return *(data() + size() - 1); }

#ifdef _GLIBCXX_STRING_VIEW
    NODISCARD INLINE operator std::basic_string_view<value_type>() const noexcept {
        return std::basic_string_view<value_type>(data(), size());
    }
#endif

    NODISCARD INLINE explicit operator bool_t() const noexcept { return !empty(); }

    template <class T, class Enable = enable_if_t<is_integral_v<T> || is_floating_point_v<T>>>
    NODISCARD INLINE constexpr T to() const {
        T result;
        auto result_pair = std::from_chars(begin(), end(), result);
        if (unlikely(result_pair.ec != std::errc{})) {
            throw bad_to();
        }
        return result;
    }
};

template <class Char, class Allocator = new_allocator_t>
class value final : public Allocator::reference {
public:
    using base = typename Allocator::reference;
    using self = value;
    using char_type = Char;
    using allocator_type = Allocator;
    using view_type = view<Char>;
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
    // #ifdef __cpp_no_unique_address
    //     typename std::
    //         conditional_t<std::is_empty_v<Allocator>, [[no_unique_address]] Allocator, Allocator>
    //             _allocator;
    // #else
    // Allocator _allocator{};
    // #endif
    Char* _impl{nullptr};
    size_type _size{0u};
    size_type _capacity{0u};

    allocator_type& _allocator() noexcept {
        using reference = typename allocator_type::reference;
        return static_cast<reference&>(*this);
    }
    allocator_type const& _allocator() const noexcept {
        using reference = typename allocator_type::reference;
        return static_cast<reference&>(*this);
    }

    template <class T, class Enable = enable_if_t<is_integral_v<T> || is_floating_point_v<T>>>
    NODISCARD INLINE static self _from(T value, size_type capacity) {
        self result{capacity};
        auto result_pair = std::to_chars(result.data(), result.data() + capacity, value);
        if (unlikely(result_pair.ec != std::errc{})) {
            throw bad_from{};
        }
        result._size = std::distance(result.data(), result_pair.ptr);
        return result;
    }

    template <class Iter1, class Iter2>
    INLINE void _assign(Iter1 begin, Iter2 end) {
        const size_type size = std::distance(begin, end);
        if (likely(size > 0)) {
            if (likely(size > capacity())) {
                _allocator().template deallocate<Char>(_impl, capacity());
                _impl = _allocator().template allocate<Char>(size + 1u);
                _capacity = size;
            }
            std::copy(begin, end, _impl);
            _impl[size] = '\0';
            _size = size;
        }
    }

public:
    INLINE constexpr value() noexcept(std::is_nothrow_constructible<allocator_type>::value) {}

    INLINE constexpr explicit value(allocator_type& allocator) noexcept(
        std::is_nothrow_constructible<allocator_type>::value)
            : base(allocator) {}

    INLINE constexpr explicit value(size_type capacity) : _capacity(capacity) {
        if (likely(capacity > 0)) {
            _impl = _allocator().template allocate<Char>(capacity + 1u);
        }
    }

    INLINE constexpr explicit value(size_type capacity, allocator_type& allocator)
            : base(allocator), _capacity(capacity) {
        if (likely(capacity > 0)) {
            _impl = _allocator().template allocate<Char>(capacity + 1u);
        }
    }

    template <class Iter1, class Iter2>
    INLINE constexpr explicit value(Iter1 begin, Iter2 end) {
        _assign(begin, end);
    }

    template <class Iter1, class Iter2>
    INLINE constexpr explicit value(Iter1 begin, Iter2 end, allocator_type& allocator)
            : base(allocator) {
        _assign(begin, end);
    }

    INLINE value(const_pointer str) : value(str, str + strlen(str)) {}

    INLINE value(const_pointer str, allocator_type& allocator)
            : value(str, str + strlen(str), allocator) {}

    INLINE value(view_type o) : value(o.begin(), o.end()) {}

    INLINE value(view_type o, allocator_type& allocator) : value(o.begin(), o.end(), allocator) {}

    INLINE value(self const& o) : base(o) { _assign(o.begin(), o.end()); }

    INLINE value(self&& o) noexcept
            : base(std::move(o)), _impl(o._impl), _size(o._size), _capacity(o._capacity) {
        o._impl = nullptr;
    }

#ifdef _GLIBCXX_STRING_VIEW
    INLINE explicit value(std::basic_string_view<Char> str)
            : value(str.data(), str.data() + str.size()) {}
#endif

    INLINE ~value() noexcept(std::is_nothrow_destructible<allocator_type>::value) {
        if (_impl != nullptr) {
            _allocator().template deallocate<Char>(_impl, _capacity);
            _impl = nullptr;
            _size = 0u;
            _capacity = 0u;
        }
    }

    INLINE self& operator=(const_pointer o) {
        _assign(o, o + strlen(o));
        return *this;
    }

    INLINE self& operator=(view_type o) {
        _assign(o.begin(), o.end());
        return *this;
    }

    INLINE self& operator=(self const& o) {
        if (unlikely(this != &o)) {
            _assign(o.begin(), o.end());
        }
        return *this;
    }

    INLINE self& operator=(self&& o) noexcept {
        if (unlikely(this != &o)) {
            _allocator().template deallocate<Char>(_impl, _capacity);
            _impl = o._impl;
            _size = o._size;
            _capacity = o._capacity;
            o._impl = nullptr;
            o._size = 0u;
            o._capacity = 0u;
        }
        return *this;
    }

    INLINE void reserve(size_type capacity) {
        if (capacity > _capacity) {
            auto impl = _allocator().template allocate<Char>(capacity + 1u);
            std::copy(begin(), end(), impl);
            _allocator().template deallocate<Char>(_impl, _capacity);
            _impl = impl;
            _impl[_size] = '\0';
            _capacity = capacity;
        }
    }

    NODISCARD INLINE constexpr pointer data() noexcept { return _impl; }

    NODISCARD INLINE constexpr const_pointer data() const noexcept { return _impl; }

    NODISCARD INLINE constexpr size_type size() const noexcept { return _size; }

    NODISCARD INLINE constexpr size_type capacity() const noexcept { return _capacity; }

    NODISCARD INLINE constexpr bool_t empty() const noexcept { return size() == 0u; }

    NODISCARD INLINE constexpr reference operator[](size_type pos) {
        if (unlikely(pos >= size())) {
            throw out_of_range{};
        }
        return data()[pos];
    }

    NODISCARD INLINE constexpr const_reference operator[](size_type pos) const {
        if (unlikely(pos >= size())) {
            throw out_of_range{};
        }
        return data()[pos];
    }

    NODISCARD INLINE constexpr iterator begin() noexcept { return data(); }

    NODISCARD INLINE constexpr const_iterator begin() const noexcept { return data(); }

    NODISCARD INLINE constexpr iterator end() noexcept { return data() + size(); }

    NODISCARD INLINE constexpr const_iterator end() const noexcept { return data() + size(); }

    NODISCARD INLINE constexpr reference front() noexcept {
        if (unlikely(size() <= 0u)) {
            throw out_of_range{};
        }
        return *data();
    }
    NODISCARD INLINE constexpr const_reference front() const noexcept {
        if (unlikely(size() <= 0u)) {
            throw out_of_range{};
        }
        return *data();
    }
    NODISCARD INLINE constexpr reference back() noexcept {
        if (unlikely(size() <= 0u)) {
            throw out_of_range{};
        }
        return *(data() + size() - 1);
    }
    NODISCARD INLINE constexpr const_reference back() const noexcept {
        if (unlikely(size() <= 0u)) {
            throw out_of_range{};
        }
        return *(data() + size() - 1);
    }

    NODISCARD INLINE constexpr const_pointer c_str() const noexcept { return data(); }

    NODISCARD INLINE constexpr bool_t starts_with(self const& o) const noexcept {
        return static_cast<view_type>(*this).starts_with(o);
    }

    NODISCARD INLINE constexpr bool_t ends_with(self const& o) const noexcept {
        return static_cast<view_type>(*this).ends_with(o);
    }

    NODISCARD INLINE constexpr bool_t operator==(view_type o) const noexcept {
        return size() == o.size() && std::equal(begin(), end(), o.begin());
    }

    template <class T>
    NODISCARD INLINE constexpr bool_t operator!=(T o) const noexcept {
        return !(*this == o);
    }

    template <class Iter1, class Iter2>
    INLINE void emplace(Iter1 __begin, Iter2 __end) {
        const size_type __size = std::distance(__begin, __end);
        if (likely(__size > 0u)) {
            const size_type __new_size = size() + __size;
            if (unlikely(__new_size > capacity())) {
                size_type const __new_capacity = __new_size * 2;
                auto __impl = _allocator().template allocate<Char>(__new_capacity + 1u);
                std::copy(begin(), end(), __impl);
                _allocator().template deallocate<Char>(_impl, _capacity);
                _impl = __impl;
                _capacity = __new_capacity;
            }
            std::copy(__begin, __end, end());
            _impl[__new_size] = '\0';
            _size = __new_size;
        }
    }

    INLINE self& operator<<(Char ch) {
        emplace(&ch, &ch + 1);
        return *this;
    }

    INLINE self& operator<<(const_pointer o) {
        emplace(o, o + strlen(o));
        return *this;
    }

    INLINE self& operator<<(view_type o) {
        emplace(o.begin(), o.end());
        return *this;
    }

    INLINE self& operator<<(self const& o) {
        emplace(o.begin(), o.end());
        return *this;
    }

    NODISCARD INLINE operator view_type() const noexcept { return view_type(begin(), end()); }

#ifdef _BASIC_STRING_H
    NODISCARD INLINE operator std::basic_string<Char>() const noexcept {
        return std::basic_string<Char>(data());
    }
#endif

    NODISCARD INLINE static self from(int8_t value) { return _from(value, 3u); }
    NODISCARD INLINE static self from(int16_t value) { return _from(value, 5u); }
    NODISCARD INLINE static self from(int32_t value) { return _from(value, 10u); }
    NODISCARD INLINE static self from(int64_t value) { return _from(value, 20u); }
    NODISCARD INLINE static self from(uint8_t value) { return _from(value, 3u); }
    NODISCARD INLINE static self from(uint16_t value) { return _from(value, 5u); }
    NODISCARD INLINE static self from(uint32_t value) { return _from(value, 10u); }
    NODISCARD INLINE static self from(uint64_t value) { return _from(value, 20u); }
    NODISCARD INLINE static self from(float32_t value) { return _from(value, 58u); }
    NODISCARD INLINE static self from(float64_t value) { return _from(value, 328u); }

    template <class T>
    NODISCARD INLINE T to() const {
        return static_cast<view_type>(*this).template to<T>();
    }

    NODISCARD INLINE static self move(Char* value, size_type size) {
        self result;
        result._impl = value;
        result._size = size;
        result._capacity = size;
        return result;
    }
};

#ifdef _GLIBCXX_OSTREAM
template <class Char>
inline std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& out, view<Char> s) {
    out.write(s.data(), s.size());
    return out;
}

template <class Char, class Allocator>
inline std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& out,
                                            value<Char, Allocator> const& s) {
    out.write(s.data(), s.size());
    return out;
}
#endif

};  // namespace string

using string_view_t = string::view<char>;
using string_t = string::value<char>;

};  // namespace qlib
