#pragma once

#define JSON_IMPLEMENTATION

// #include <iostream>
// #include <array>
// #include <atomic>
// #include <immintrin.h>
#include <vector>

#include "qlib/string.h"

namespace qlib {
namespace json {

template <class T>
constexpr static inline bool_t is_number_v = is_integral_v<T> || is_floating_point_v<T>;

template <class Char>
using string_view_t = string::value<Char, memory_policy_t::view>;

template <class Char>
using string_t = string::value<Char, memory_policy_t::copy>;

template <class Char>
constexpr static inline string_view_t<Char> null_str{"null"};

template <class Char>
constexpr static inline string_view_t<Char> true_str{"true"};

template <class Char>
constexpr static inline string_view_t<Char> false_str{"false"};

enum class error : int32_t {
    unknown = -1,
    impl_nullptr = -2,
    param_invalid = -3,
    file_not_found = -4,
    file_not_support = -5,
    file_invalid = -6,
    missing_left_brace = -7,
    missing_right_brace = -8,
    missing_left_quote = -9,
    missing_right_quote = -10,
    missing_colon = -11,
    missing_comma = -12,
    invalid_unicode = -13,
    invalid_null = -14,
    invalid_boolean = -15,
};

enum class value_enum : uint8_t {
    null = 0,
    object = 1 << 0,
    array = 1 << 1,
    string = 1 << 2,
    number = 1 << 3,
    boolean = 1 << 4,
    // string_ref = 1 << 5,
    number_ref = 1 << 6,
};

class not_object final : public exception {
public:
    char const* what() const noexcept override { return "not object"; }
};

class not_array final : public exception {
public:
    char const* what() const noexcept override { return "not array"; }
};

class not_string final : public exception {
public:
    char const* what() const noexcept override { return "not string"; }
};

class not_number final : public exception {
public:
    char const* what() const noexcept override { return "not number"; }
};

class not_boolean final : public exception {
public:
    char const* what() const noexcept override { return "not boolean"; }
};

class bad_convert final : public exception {
public:
    char const* what() const noexcept override { return "bad convert"; }
};

template <class... Args>
struct storage final {
    template <class... Ts>
    static constexpr size_t max_of() {
        size_t sizes[] = {sizeof(Ts)...};
        size_t max_size = sizes[0];
        for (size_t i = 1; i < sizeof...(Ts); ++i) {
            if (sizes[i] > max_size)
                max_size = sizes[i];
        }
        return max_size;
    }

    template <class... Ts>
    static constexpr size_t max_align_of() {
        size_t aligns[] = {alignof(Ts)...};
        size_t max_align = aligns[0];
        for (size_t i = 1; i < sizeof...(Ts); ++i) {
            if (aligns[i] > max_align)
                max_align = aligns[i];
        }
        return max_align;
    }

    static constexpr auto max_size = max_of<Args...>();
    static constexpr auto max_align = max_align_of<Args...>();
    union {
        uint8_t _value[max_size];
        struct __attribute__((__aligned__(max_align))) {
        } __align;
    };
};

template <class Char, class T, class Enable = void>
struct converter;

template <class Char>
struct converter<Char, bool_t> final : public object {
    static string_view_t<Char> encode(bool_t value) {
        return value ? true_str<Char> : false_str<Char>;
    }

    static bool_t decode(string_view_t<Char> s) {
        if (s == true_str<Char>) {
            return True;
        } else if (s == false_str<Char>) {
            return False;
        } else {
            throw bad_convert();
        }
    }
};

template <class Char, class T>
struct converter<Char, T, enable_if_t<is_number_v<T>>> final : public object {
    using string_view_t = string::value<Char, memory_policy_t::view>;
    using string_t = string::value<Char, memory_policy_t::copy>;

    static string_t encode(T value) {
        try {
            return string_t::from(value);
        } catch (string::bad_from const& _) {
            throw bad_convert();
        }
    }
    static T decode(string_view_t s) {
        try {
            return s.template to<T>();
        } catch (string::bad_to const& _) {
            throw bad_convert();
        }
    }
};

template <class Char, memory_policy_t Policy>
class parser;

template <class Char, memory_policy_t Policy>
class value final : public object {
public:
    using base = object;
    using self = value<Char, Policy>;
    using char_type = Char;
    using string_view_t = json::string_view_t<Char>;
    using string_t = json::string_t<Char>;
    using key_type = string::value<Char, Policy>;
    struct pair final {
        key_type key;
        self value;

        template <class Key, class Value>
        pair(Key&& _key, Value&& _value)
                : key(std::forward<Key>(_key)), value(std::forward<Value>(_value)) {}

        pair() = default;
        pair(pair const&) = default;
        pair(pair&&) = default;
        pair& operator=(pair const&) = default;
        pair& operator=(pair&&) = default;
    };
    using object_type = std::vector<pair>;
    using array_type = std::vector<self>;
    using string_type = string::value<Char, Policy>;
    using size_type = size_t;
    constexpr static inline memory_policy_t memory_policy = Policy;
    const static inline self default_value = self{};

protected:
    value_enum _type{value_enum::null};
    using impl_type = storage<object_type, array_type, string_t, string_view_t>;
    impl_type _impl;

    friend class parser<Char, Policy>;

    struct FixedOutStream final : public object {
    protected:
        Char* _impl{nullptr};
        size_type _size{0u};
        size_type _capacity{0u};

    public:
        explicit FixedOutStream(size_type capacity)
                : _impl(new Char[capacity + 1u]), _size(0u), _capacity(capacity) {}

        FixedOutStream(FixedOutStream const&) = delete;
        FixedOutStream& operator=(FixedOutStream const&) = delete;
        FixedOutStream(FixedOutStream&& o)
                : _impl(o._impl), _size(o._size), _capacity(o._capacity) {
            o._impl = nullptr;
            o._size = 0u;
            o._capacity = 0u;
        }

        FixedOutStream& operator=(FixedOutStream&& o) noexcept {
            if (this != &o) {
                if (_impl != nullptr) {
                    delete[] _impl;
                }
                _impl = o._impl;
                _size = o._size;
                _capacity = o._capacity;
                o._impl = nullptr;
                o._size = 0u;
                o._capacity = 0u;
            }
            return *this;
        }

        ~FixedOutStream() {
            if (_impl != nullptr) {
                delete[] _impl;
                _impl = nullptr;
                _size = 0u;
                _capacity = 0u;
            }
        }

        FixedOutStream& operator<<(string_view_t s) {
            size_type new_size = _size + s.size();
            if (unlikely(new_size > _capacity)) {
                throw exception();
            }
            std::copy(s.begin(), s.end(), _impl + _size);
            _size = new_size;
            return *this;
        }

        [[nodiscard]] bool_t operator==(string_view_t s) const {
            return _size == s.size() && std::equal(s.begin(), s.end(), _impl);
        }
    };

    template <class Iter1, class Iter2>
    static inline constexpr int32_t _parse_unicode(uint32_t& code, Iter1& begin, Iter2 end) {
        int32_t result{0u};
        do {
            code = 0;
            for (uint8_t i = 0; i < 4; ++i) {
                Char c = *begin;
                code <<= 4;
                if (c >= '0' && c <= '9') {
                    code |= (c - '0');
                } else if (c >= 'A' && c <= 'F') {
                    code |= (c - 'A' + 10);
                } else if (c >= 'a' && c <= 'f') {
                    code |= (c - 'a' + 10);
                } else {
                    result = -1;
                    break;
                }
                ++begin;
            }
            if (unlikely(!result && code > (0xD800 - 1) && code < 0xDC00)) {
                if (*begin != '\\' || *(begin + 1) != 'u') {
                    result = -1;
                    break;
                }
                begin += 2;  // 跳过 \u
                uint16_t code2{0u};
                for (uint8_t i = 0; i < 4; ++i) {
                    Char c = *begin;
                    code2 <<= 4;
                    if (c >= '0' && c <= '9') {
                        code2 |= (c - '0');
                    } else if (c >= 'A' && c <= 'F') {
                        code2 |= (c - 'A' + 10);
                    } else if (c >= 'a' && c <= 'f') {
                        code2 |= (c - 'a' + 10);
                    } else {
                        result = -1;
                        break;
                    }
                    ++begin;
                }
                code = 0x10000 + ((code - 0xD800) << 10) + (code2 - 0xDC00);
            }
        } while (0);
        return result;
    }

    template <class Iter1, class Iter2>
    static constexpr int32_t _parse_string(string_t* value, Iter1 begin, Iter2 end) {
        int32_t result{0u};

        auto start = begin;
        value->reserve(std::distance(begin, end));
        while (begin < end && result == 0) {
            if (*begin != '\\') {
                ++begin;
            } else {
                *value << string_view_t{start, begin};
                ++begin;
                switch (*begin) {
                    case '"':
                    case '\\':
                    case '/':
                        *value << *begin;
                        ++begin;
                        break;
                    case 'b':
                        *value << '\b';
                        ++begin;
                        break;
                    case 'f':
                        *value << '\f';
                        ++begin;
                        break;
                    case 'n':
                        *value << '\n';
                        ++begin;
                        break;
                    case 'r':
                        *value << '\r';
                        ++begin;
                        break;
                    case 't':
                        *value << '\t';
                        ++begin;
                        break;
                    case 'u': {
                        ++begin;
                        uint32_t code{0u};
                        result = _parse_unicode(code, begin, end);
                        if (0 != result) {
                            break;
                        }
                        if (code <= 0x7f) {
                            *value << (char)code;
                        } else if (code <= 0x7ff) {
                            *value << (Char)(0xc0 | (code >> 6));
                            *value << (Char)(0x80 | (code & 0x3f));
                        } else if (code <= 0xffff) {
                            *value << (Char)(0xe0 | (code >> 12));
                            *value << (Char)(0x80 | ((code >> 6) & 0x3f));
                            *value << (Char)(0x80 | (code & 0x3f));
                        } else if (code <= 0x10ffff) {
                            *value << (Char)(0xf0 | (code >> 18));
                            *value << (Char)(0x80 | ((code >> 12) & 0x3f));
                            *value << (Char)(0x80 | ((code >> 6) & 0x3f));
                            *value << (Char)(0x80 | (code & 0x3f));
                        } else {
                            result = -1;
                        }
                        break;
                    }
                    default: {
                        *value << *begin;
                        ++begin;
                    }
                }
                start = begin;
            }
        }
        if (!result) {
            *value << string_view_t{start, begin};
        }
        return result;
    }

    static constexpr string_t _convert(string_view_t s) {
        string_t result;
        if (_parse_string(&result, s.begin(), s.end())) {
            throw not_string{};
        }
        return result;
    }

public:
    template <class T>
    class value_ref final : public object {
    public:
        using self = value_ref;
        using value_type = T;

    protected:
        mutable T _impl;

    public:
        value_ref() = delete;
        value_ref(self const&) = delete;
        value_ref(self&&) = default;
        self& operator=(self const&) = delete;
        self& operator=(self&&) = default;

        template <class... Args>
        value_ref(Args&&... args) : _impl(std::forward<Args>(args)...) {}

        value_type operator*() const noexcept { return std::move(_impl); }
    };

    constexpr value() noexcept = default;

    constexpr value(object_type const& value) : _type(value_enum::object) {
        new (&_impl) object_type(value);
    }

    constexpr value(object_type&& value) : _type(value_enum::object) {
        new (&_impl) object_type(std::move(value));
    }

    constexpr value(array_type const& value) : _type(value_enum::array) {
        new (&_impl) array_type(value);
    }

    constexpr value(array_type&& value) : _type(value_enum::array) {
        new (&_impl) array_type(std::move(value));
    }

    constexpr value(Char const* value) : _type(value_enum::string) {
        new (&_impl) string_type(value);
    }

    constexpr value(string_type const& value) : _type(value_enum::string) {
        new (&_impl) string_type(value);
    }

    constexpr value(string_type&& value) : _type(value_enum::string) {
        new (&_impl) string_type(std::move(value));
    }

    template <class T, class Enable = enable_if_t<is_number_v<T>>>
    constexpr value(T value) : _type(value_enum::number) {
        new (&_impl) string_t(converter<Char, T>::encode(value));
    }

    constexpr value(bool_t value) : _type(value_enum::boolean) {
        new (&_impl) string_view_t(converter<Char, bool_t>::encode(value));
    }

    constexpr value(self const& o) : _type(o._type) {
        switch (_type) {
            case value_enum::object: {
                new (&_impl) object_type(*(object_type*)(&o._impl));
                break;
            }
            case value_enum::array: {
                new (&_impl) array_type(*(array_type*)(&o._impl));
                break;
            }
            case value_enum::string: {
                new (&_impl) string_type(*(string_type*)(&o._impl));
                break;
            }
            case value_enum::number: {
                new (&_impl) string_t(*(string_t*)(&o._impl));
                break;
            }
            case value_enum::boolean: {
                new (&_impl) string_view_t(*(string_view_t*)(&o._impl));
                break;
            }
            case value_enum::number_ref: {
                new (&_impl) string_type(*(string_type*)(&o._impl));
                break;
            }
            default:;
        }
    }

    constexpr value(self&& o) : _type(o._type), _impl(std::move(o._impl)) {
        o._type = value_enum::null;
    }

    constexpr value(std::initializer_list<value_ref<pair>> list) : _type(value_enum::object) {
        new (&_impl) object_type();
        auto object = (object_type*)(&_impl);
        object->reserve(list.size());
        for (auto const& item : list) {
            object->emplace_back(std::move(*item));
        }
    }

    ~value() noexcept {
        switch (_type) {
            case value_enum::object: {
                ((object_type*)&_impl)->~object_type();
                break;
            }
            case value_enum::array: {
                ((array_type*)&_impl)->~array_type();
                break;
            }
            case value_enum::string: {
                ((string_type*)&_impl)->~string_type();
                break;
            }
            case value_enum::number: {
                ((string_t*)&_impl)->~string_t();
                break;
            }
            case value_enum::boolean: {
                ((string_view_t*)&_impl)->~string_view_t();
                break;
            }
            case value_enum::number_ref: {
                ((string_type*)&_impl)->~string_type();
                break;
            }
            default:;
        }
    }

    template <class T>
    self& operator=(T&& o) {
        this->~value();
        new (this) self(std::forward<T>(o));
        return *this;
    }

    self& operator=(self const& o) {
        if (unlikely(this != &o)) {
            this->~value();
            new (this) self(o);
        }
        return *this;
    }

    self& operator=(self&& o) {
        if (unlikely(this != &o)) {
            this->~value();
            new (this) self(std::move(o));
        }
        return *this;
    }

    [[nodiscard]] constexpr bool_t empty() const noexcept { return _type == value_enum::null; }

    [[nodiscard]] constexpr auto type() const noexcept { return _type; }

    template <class T>
    [[nodiscard]] constexpr T get() const {
        if constexpr (is_number_v<T>) {
            if (likely(_type == value_enum::number_ref)) {
                return converter<Char, T>::decode(*(string_type*)(&_impl));
            } else if (likely(_type == value_enum::number)) {
                return converter<Char, T>::decode(*(string_t*)(&_impl));
            } else {
                throw not_number();
            }
        } else if constexpr (is_same_v<T, bool_t>) {
            if (likely(_type == value_enum::boolean)) {
                return converter<Char, T>::decode(*(string_view_t*)(&_impl));
            } else {
                throw not_boolean();
            }
        } else if constexpr (is_same_v<T, string_view_t>) {
            if (_type == value_enum::string) {
                return *(string_type*)(&_impl);
            } else {
                throw not_string();
            }
        } else if constexpr (is_same_v<T, string_t>) {
            if (_type == value_enum::string) {
                return _convert(*(string_type*)(&_impl));
            } else {
                throw not_string();
            }
        } else {
            static_assert(is_number_v<T> || is_same_v<T, bool_t> || is_same_v<T, string_t> ||
                              is_same_v<T, string_view_t>,
                          "unsupported type");
            throw not_number();  // 这行不会执行，但为了通过编译检查
        }
    }

    template <class T>
    [[nodiscard]] constexpr T get(T&& default_value) const {
        if (empty()) {
            return std::forward<T>(default_value);
        }
        return get<T>();
    }

    [[nodiscard]] constexpr self const& operator[](string_view_t key) const {
        for (auto const& pair : object()) {
            if (pair.key == key) {
                return pair.value;
            }
        }
        return default_value;
    }

    [[nodiscard]] constexpr self& operator[](string_view_t key) {
        auto& object = this->object();
        for (auto& pair : object) {
            if (pair.key == key) {
                return pair.value;
            }
        }
        object.emplace_back(key, self());
        return object.back().value;
    }

    [[nodiscard]] object_type& object() {
        if (unlikely(_type != value_enum::object)) {
            throw not_object();
        }
        return *(object_type*)(&_impl);
    }
    [[nodiscard]] object_type const& object() const { return const_cast<self&>(*this).object(); }

    [[nodiscard]] array_type& array() {
        if (unlikely(_type != value_enum::array)) {
            throw not_array{};
        }
        auto impl = (array_type*)(&_impl);
        return *impl;
    }

    [[nodiscard]] array_type const& array() const { return const_cast<self&>(*this).array(); }

    [[nodiscard]] static self object(std::initializer_list<value_ref<pair>> list) {
        self value;
        value._type = value_enum::object;
        new (&value._impl) object_type();
        auto object = (object_type*)(&value._impl);
        object->reserve(list.size());
        for (auto const& item : list) {
            object->emplace_back(std::move(*item));
        }
        return value;
    }

    [[nodiscard]] static self array(std::initializer_list<value_ref<self>> list) {
        self value;
        value._type = value_enum::array;
        new (&value._impl) array_type();
        auto array = (array_type*)(&value._impl);
        array->reserve(list.size());
        for (auto const& item : list) {
            array->emplace_back(std::move(*item));
        }
        return value;
    }

    template <class OutStream>
    constexpr OutStream& to(OutStream& out) const {
        switch (_type) {
            case value_enum::null: {
                out << null_str<Char>;
                break;
            }
            case value_enum::string: {
                out << "\"" << *(string_type*)(&_impl) << "\"";
                break;
            }
            case value_enum::number: {
                out << *((string_t*)(&_impl));
                break;
            }
            case value_enum::number_ref: {
                out << *((string_type*)(&_impl));
                break;
            }
            case value_enum::boolean: {
                out << *((string_view_t*)(&_impl));
                break;
            }
            case value_enum::array: {
                out << "[";
                auto& array = this->array();
                for (auto it = array.begin(); it != array.end();) {
                    out << *it;
                    ++it;
                    if (likely(it != array.end())) {
                        out << ",";
                    }
                }
                out << "]";
                break;
            }
            case value_enum::object: {
                out << "{";
                auto& object = this->object();
                for (auto it = object.begin(); it != object.end();) {
                    out << "\"" << it->key << "\"" << ":" << it->value;
                    ++it;
                    if (likely(it != object.end())) {
                        out << ",";
                    }
                }
                out << "}";
                break;
            }
            default:;
        }
        return out;
    }

    [[nodiscard]] constexpr auto to() const {
        string_t out(1024u);
        return to(out);
    }

    [[nodiscard]] explicit operator bool_t() const noexcept { return !empty(); }

    [[nodiscard]] bool_t operator==(string_view_t text) const {
        FixedOutStream out(text.size());
        bool_t ok{False};
        try {
            ok = (to(out) == text);
        } catch (exception const& _) {
            ok = False;
        }
        return ok;
    }

    template <class T>
    [[nodiscard]] bool_t operator!=(T const& o) const {
        return !(*this == o);
    }
};

template <class Char, memory_policy_t Policy>
class parser final : public object {
public:
    using size_type = size_t;
    using json_type = value<Char, Policy>;
    using string_type = typename json_type::string_type;
    using array_type = typename json_type::array_type;
    using object_type = typename json_type::object_type;
    using key_type = typename json_type::key_type;
    using string_t = typename json_type::string_t;
    using string_view_t = typename json_type::string_view_t;

protected:
    size_type _capacity{16u};

    struct impl {
        bool_t is_object;
        storage<object_type, array_type> values;
        key_type key;
    };
    using impl_type = impl;

    int32_t _error{0u};

    // struct pool_entry {
    //     std::atomic<bool_t> is_used{False};
    //     std::vector<impl_type> layers{};
    // };

    // static inline thread_local std::array<pool_entry, 4u> thread_pool{};
    // static inline std::array<pool_entry, 16u> pool{};

    // static inline bool_t _init_pool = []() {
    //     for (auto& it : pool) {
    //         it.is_used = False;
    //         it.layers.reserve(32u);
    //     }
    //     for (auto& it : thread_pool) {
    //         it.is_used = False;
    //         it.layers.reserve(32u);
    //     }
    //     return True;
    // }();

    static auto& _is_object_from_type(impl_type& value) { return value.is_object; }

    // static auto& _key_from_type(impl_type& value) {
    //     return *(key_type*)((uint8_t*)&value + offsetof(impl, key));
    // }

    static auto& _key_from_type(impl_type& value) { return value.key; }

    // static auto& _object_from_type(impl_type& value) {
    //     return *(object_type*)((uint8_t*)&value + offsetof(impl, values));
    // }

    static auto& _object_from_type(impl_type& value) { return *(object_type*)(&value.values); }

    // static auto& _array_from_type(impl_type& value) {
    //     return *(array_type*)((uint8_t*)&value + offsetof(impl, values));
    // }

    static auto& _array_from_type(impl_type& value) { return *(array_type*)(&value.values); }

    inline auto _impl_init(impl_type& value, bool_t is_object) {
        _is_object_from_type(value) = is_object;
        if (is_object) {
            auto& object = _object_from_type(value);
            new (&object) object_type();
            object.reserve(_capacity);
        } else {
            auto& array = _array_from_type(value);
            new (&array) array_type();
            array.reserve(_capacity);
        }
    }

    inline auto _impl_init(impl_type& value, bool_t is_object, string_view_t key) {
        _impl_init(value, is_object);
        auto& _key = _key_from_type(value);
        new (&_key) key_type(key);
    }

    inline auto _impl_emplace(impl_type& impl, string_view_t key, json_type&& value) {
        auto& object = _object_from_type(impl);
        object.emplace_back(key, std::move(value));
    }

    inline auto _impl_emplace(impl_type& impl, json_type&& value) {
        auto& array = _array_from_type(impl);
        array.emplace_back(std::move(value));
    }

    static inline constexpr bool_t skip_table[256] = {
        1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,  // \t, \n, \r
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,  // ' ', ','
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
        // ... 其余为0
    };

    static inline constexpr bool_t end_table[256] = {
        1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,  // 0-15   (\t=9, \n=10, \r=13)
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 16-31
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,  // 32-47  (space=32, ','=44)
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 48-63
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 64-79
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,  // 80-95  (']'=93)
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 96-111
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,  // 112-127 ('}'=125)
    };

    // static inline constexpr Char escape_char_map[256] = {
    //     0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0, 0, 0,    0, 0, 0,    // 0-15
    //     0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0, 0, 0,    0, 0, 0,    // 16-31
    //     0, 0, '"',  0, 0, 0, 0,    0, 0, 0, 0, 0, 0,    0, 0, '/',  // 32-47 ('"'=34, '/'=47)
    //     0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0, 0, 0,    0, 0, 0,    // 48-63
    //     0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0, 0, 0,    0, 0, 0,    // 64-79
    //     0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0, 0, '\\', 0, 0, 0,    // 80-95 ('\\'=92)
    //     0, 0, '\b', 0, 0, 0, '\f', 0, 0, 0, 0, 0, 0,    0, 0, 0,    // 96-111
    //     0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0, 0, 0,    0, 0, 0,    // 112-127
    // };

    // static inline constexpr bool_t hex_char_table[256] = {
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0-15
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 16-31
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 32-47
    //     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  // 48-63  (0-9)
    //     0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 64-79  (A-F)
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 80-95
    //     0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 96-111 (a-f)
    //     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 112-127
    // };

    // template <class Iter1, class Iter2>
    // inline constexpr Iter1 _parse_unicode(uint32_t& code, Iter1 begin, Iter2 end) {
    //     do {
    //         code = 0;
    //         for (uint8_t i = 0; i < 4; ++i) {
    //             Char c = *begin;
    //             if (!hex_char_table[(uint8_t)c]) {
    //                 _error = (int32_t)error::invalid_unicode;
    //                 break;
    //             }
    //             code <<= 4;
    //             if (c >= '0' && c <= '9') {
    //                 code |= (c - '0');
    //             } else if (c >= 'A' && c <= 'F') {
    //                 code |= (c - 'A' + 10);
    //             } else if (c >= 'a' && c <= 'f') {
    //                 code |= (c - 'a' + 10);
    //             }
    //             ++begin;
    //         }
    //         if (likely(!_error)) {
    //             if (unlikely(code > (0xD800 - 1) && code < 0xDC00)) {
    //                 if (*begin != '\\' || *(begin + 1) != 'u') {
    //                     _error = (int32_t)error::invalid_unicode;
    //                     break;
    //                 }
    //                 begin += 2;  // 跳过 \u
    //                 uint16_t code2{0u};
    //                 for (uint8_t i = 0; i < 4; ++i) {
    //                     Char c = *begin;
    //                     if (!hex_char_table[(uint8_t)c]) {
    //                         _error = (int32_t)error::invalid_unicode;
    //                         break;
    //                     }
    //                     code2 <<= 4;
    //                     if (c >= '0' && c <= '9') {
    //                         code2 |= (c - '0');
    //                     } else if (c >= 'A' && c <= 'F') {
    //                         code2 |= (c - 'A' + 10);
    //                     } else if (c >= 'a' && c <= 'f') {
    //                         code2 |= (c - 'a' + 10);
    //                     }
    //                     ++begin;
    //                 }
    //                 code = 0x10000 + ((code - 0xD800) << 10) + (code2 - 0xDC00);
    //             }
    //         }
    //     } while (0);
    //     return begin;
    // }

    // template <class Iter1, class Iter2>
    // constexpr Iter1 _parse_string(string_wrapper* value, Iter1 begin, Iter2 end) {
    //     ++begin;
    //     auto start = begin;
    //     value->is_view = True;
    //     while (begin < end && !_error) {
    //         if (likely(*begin != '\\')) {
    //             if (*begin == '"') {
    //                 if (value->is_view) {
    //                     value->ptr = start;
    //                     value->size = std::distance(start, begin);
    //                 } else {
    //                     value->ptr[value->size] = '\0';
    //                 }
    //                 break;
    //             }
    //             ++begin;
    //         } else {
    //             if (value->is_view) {
    //                 value->ptr = new Char[std::distance(start, end)];
    //                 std::copy(start, begin, value->ptr);
    //                 value->size = std::distance(start, begin);
    //                 value->is_view = False;
    //             }
    //             ++begin;
    //             switch (*begin) {
    //                 case '"':
    //                     ++begin;
    //                     value->ptr[value->size++] = '"';
    //                     break;
    //                 case '\\':
    //                     ++begin;
    //                     value->ptr[value->size++] = '\\';
    //                     break;
    //                 case '/':
    //                     ++begin;
    //                     value->ptr[value->size++] = '/';
    //                     break;
    //                 case 'b':
    //                     ++begin;
    //                     value->ptr[value->size++] = '\b';
    //                     break;
    //                 case 'f':
    //                     ++begin;
    //                     value->ptr[value->size++] = '\f';
    //                     break;
    //                 case 'n':
    //                     ++begin;
    //                     value->ptr[value->size++] = '\n';
    //                     break;
    //                 case 'r':
    //                     ++begin;
    //                     value->ptr[value->size++] = '\r';
    //                     break;
    //                 case 't':
    //                     ++begin;
    //                     value->ptr[value->size++] = '\t';
    //                     break;
    //                 case 'u': {
    //                     ++begin;
    //                     uint32_t code{0u};
    //                     begin = _parse_unicode(code, begin, end);
    //                     if (likely(!_error)) {
    //                         if (code <= 0x7f) {
    //                             value->ptr[value->size++] = (Char)code;
    //                         } else if (code <= 0x7ff) {
    //                             value->ptr[value->size++] = (Char)(0xc0 | (code >> 6));
    //                             value->ptr[value->size++] = (Char)(0x80 | (code & 0x3f));
    //                         } else if (code <= 0xffff) {
    //                             value->ptr[value->size++] = (Char)(0xe0 | (code >> 12));
    //                             value->ptr[value->size++] = (Char)(0x80 | ((code >> 6) & 0x3f));
    //                             value->ptr[value->size++] = (Char)(0x80 | (code & 0x3f));
    //                         } else if (code <= 0x10ffff) {
    //                             value->ptr[value->size++] = (Char)(0xf0 | (code >> 18));
    //                             value->ptr[value->size++] = (Char)(0x80 | ((code >> 12) & 0x3f));
    //                             value->ptr[value->size++] = (Char)(0x80 | ((code >> 6) & 0x3f));
    //                             value->ptr[value->size++] = (Char)(0x80 | (code & 0x3f));
    //                         } else {
    //                             _error = (int32_t)error::invalid_unicode;
    //                             return begin;
    //                         }
    //                     }
    //                 }
    //                 default: {
    //                     ++begin;
    //                     value->ptr[value->size++] = *begin;
    //                     break;
    //                 }
    //             }
    //         }
    //     }
    //     ++begin;
    //     return begin;
    // }

    // // 使用SSE优化的跳过空白字符函数
    // template <class Iter1, class Iter2>
    // constexpr inline Iter1 _skip_whitespace_sse(Iter1 begin, Iter2 end) {
    //     // 检查是否支持足够的数据进行SIMD处理
    //     if (std::distance(begin, end) >= 16) {
    //         // 预定义的空白字符掩码 (基于skip_table)
    //         // \t=9, \n=10, \r=13, space=32, ','=44
    //         alignas(16) static constexpr uint8_t whitespace_mask[16] = {
    //             0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0  // 前16个字符
    //         };

    //         // 加载掩码到SSE寄存器
    //         __m128i mask = _mm_load_si128((__m128i const*)whitespace_mask);

    //         while (std::distance(begin, end) >= 16) {
    //             // 加载16个字符
    //             __m128i data = _mm_loadu_si128((__m128i const*)begin);

    //             // 查找空白字符 (使用shuffle技术)
    //             __m128i shuffled = _mm_shuffle_epi8(mask, data);
    //             __m128i is_whitespace = _mm_cmpeq_epi8(shuffled, _mm_set1_epi8(1));

    //             // 检查是否有非空白字符
    //             int mask_result = _mm_movemask_epi8(is_whitespace);
    //             if (mask_result != 0xFFFF) {  // 如果不是所有字符都是空白
    //                 // 找到第一个非空白字符的位置
    //                 int first_non_whitespace = __builtin_ctz(~mask_result);
    //                 std::advance(begin, first_non_whitespace);
    //                 break;
    //             }

    //             // 全部是空白字符，跳过16个字符
    //             std::advance(begin, 16);
    //         }
    //     }

    //     // 处理剩余字符或小数据集
    //     while (begin < end && skip_table[(uint8_t)*begin]) {
    //         ++begin;
    //     }

    //     return begin;
    // }

    template <class Iter1, class Iter2>
    constexpr inline Iter1 _parse_string(string_view_t* value, Iter1 begin, Iter2 end) {
        ++begin;

        // bool_t find{False};
        auto start = begin;

        // __m128i escapes = _mm_set1_epi8('\\');
        // __m128i quotes = _mm_set1_epi8('"');
        // while (begin < end - 15) {
        //     __m128i chunk = _mm_loadu_si128((__m128i*)begin);
        //     __m128i has_escape = _mm_cmpeq_epi8(chunk, escapes);
        //     auto has_escape_mask = _mm_movemask_epi8(has_escape);
        //     if (has_escape_mask != 0x0000) {
        //         auto first_escape = __builtin_ctz(~has_escape_mask);
        //         begin += first_escape + 2;
        //     } else {
        //         __m128i has_quote = _mm_cmpeq_epi8(chunk, quotes);
        //         auto has_quote_mask = _mm_movemask_epi8(has_quote);
        //         if (has_quote_mask != 0x0000) {
        //             auto first_quote = __builtin_ctz(has_quote_mask);
        //             begin += first_quote;
        //             *value = string_view_t(start, begin);
        //             ++begin;
        //             find = True;
        //             break;
        //         }
        //         begin += 16;
        //     }
        // }

        // if (!find) {
        while (begin < end) {
            if (*begin == '\\') {
                ++begin;
            } else if (*begin == '"') {
                *value = string_view_t(start, begin);
                ++begin;
                break;
            }
            ++begin;
        }
        // }

        return begin;
    }

    template <class Iter1, class Iter2>
    constexpr int32_t _call(json_type* json,
                            Iter1 begin,
                            Iter2 end,
                            std::vector<impl_type>& layers) {
        while (begin < end && !_error) {
            while (begin < end && skip_table[(uint8_t)*begin]) {
                ++begin;
            }

            if (!layers.empty() && _is_object_from_type(layers.back())) {
                string_view_t key;
                begin = _parse_string(&key, begin, end);
                while (begin < end && skip_table[(uint8_t)*begin]) {
                    ++begin;
                }
                switch (*begin) {
                    case '"': {
                        string_view_t value;
                        begin = _parse_string(&value, begin, end);
                        _impl_emplace(layers.back(), key, json_type(value));
                        break;
                    }
                    case '[': {
                        layers.emplace_back(impl_type{});
                        _impl_init(layers.back(), False, key);
                        ++begin;
                        break;
                    }
                    case '{': {
                        layers.emplace_back(impl_type{});
                        _impl_init(layers.back(), True, key);
                        ++begin;
                        break;
                    }
                    case 'n': {
                        if (likely(*(begin + 1) == 'u' && *(begin + 2) == 'l' &&
                                   *(begin + 3) == 'l')) {
                            _impl_emplace(layers.back(), key, {});
                        } else {
                            _error = (int32_t)error::invalid_null;
                        }
                        begin += 4;
                        break;
                    }
                    case 't': {
                        if (likely(*(begin + 1) == 'r' && *(begin + 2) == 'u' &&
                                   *(begin + 3) == 'e')) {
                            _impl_emplace(layers.back(), key, true);
                        } else {
                            _error = (int32_t)error::invalid_boolean;
                        }
                        begin += 4;
                        break;
                    }
                    case 'f': {
                        if (likely(begin[1] == 'a' && begin[2] == 'l' && begin[3] == 's' &&
                                   begin[4] == 'e')) {
                            _impl_emplace(layers.back(), key, false);
                        } else {
                            _error = (int32_t)error::invalid_boolean;
                        }
                        begin += 5;
                        break;
                    }
                    default: {
                        auto start = begin;
                        while (begin < end && !end_table[(uint8_t)*begin]) {
                            ++begin;
                        }
                        auto stop = begin;
                        if (likely(stop > start)) {
                            json_type json_value;
                            json_value._type = value_enum::number_ref;
                            new (&json_value._impl) string_type(start, stop);
                            _impl_emplace(layers.back(), key, std::move(json_value));
                        }
                    }
                }
            } else {
                switch (*begin) {
                    case '"': {
                        string_view_t value;
                        begin = _parse_string(&value, begin, end);
                        _impl_emplace(layers.back(), json_type(value));
                        break;
                    }
                    case '[': {
                        layers.emplace_back(impl_type{});
                        _impl_init(layers.back(), False);
                        ++begin;
                        continue;
                    }
                    case '{': {
                        layers.emplace_back(impl_type{});
                        _impl_init(layers.back(), True);
                        ++begin;
                        continue;
                    }
                    case 'n': {
                        if (likely(*(begin + 1) == 'u' && *(begin + 2) == 'l' &&
                                   *(begin + 3) == 'l')) {
                            _impl_emplace(layers.back(), {});
                        } else {
                            _error = (int32_t)error::invalid_null;
                        }
                        begin += 4;
                        break;
                    }
                    case 't': {
                        if (likely(*(begin + 1) == 'r' && *(begin + 2) == 'u' &&
                                   *(begin + 3) == 'e')) {
                            _impl_emplace(layers.back(), true);
                        } else {
                            _error = (int32_t)error::invalid_boolean;
                        }
                        begin += 4;
                        break;
                    }
                    case 'f': {
                        if (likely(begin[1] == 'a' && begin[2] == 'l' && begin[3] == 's' &&
                                   begin[4] == 'e')) {
                            _impl_emplace(layers.back(), false);
                        } else {
                            _error = (int32_t)error::invalid_boolean;
                        }
                        begin += 5;
                        break;
                    }
                    default: {
                        auto start = begin;
                        while (begin < end && !end_table[(uint8_t)*begin]) {
                            ++begin;
                        }
                        auto stop = begin;
                        if (likely(stop > start)) {
                            json_type json_value;

                            json_value._type = value_enum::number_ref;
                            new (&json_value._impl) string_type(start, stop);

                            _impl_emplace(layers.back(), std::move(json_value));
                        }
                    }
                }
            }

            while (begin < end) {
                if (skip_table[(uint8_t)*begin]) {
                    ++begin;
                } else if (*begin == '}') {
                    if (likely(layers.size() > 1u)) {
                        auto& object = _object_from_type(layers.back());
                        json_type value(std::move(object));
                        auto& __last_layer = layers[layers.size() - 2];
                        if (_is_object_from_type(__last_layer)) {
                            _impl_emplace(__last_layer, _key_from_type(layers.back()),
                                          std::move(value));
                        } else {
                            _impl_emplace(__last_layer, std::move(value));
                        }
                        layers.pop_back();
                    } else {
                        auto& object = _object_from_type(layers.front());
                        *json = std::move(object);
                    }
                    ++begin;
                } else if (*begin == ']') {
                    if (likely(layers.size() > 1)) {
                        json_type value;
                        value._type = value_enum::array;
                        auto& array = _array_from_type(layers.back());
                        new (&value._impl) array_type(std::move(array));
                        auto& __last_layer = layers[layers.size() - 2];
                        if (_is_object_from_type(__last_layer)) {
                            _impl_emplace(__last_layer, _key_from_type(layers.back()),
                                          std::move(value));
                        } else {
                            _impl_emplace(__last_layer, std::move(value));
                        }
                        layers.pop_back();
                    } else {
                        auto& array = _array_from_type(layers.front());
                        *json = std::move(array);
                    }
                    ++begin;
                } else {
                    break;
                }
            }
        }

        return _error;
    }

public:
    constexpr parser() noexcept = default;

    constexpr parser(size_type capacity) noexcept : _capacity(capacity) {}

    template <class Iter1, class Iter2>
    int32_t operator()(json_type* json, Iter1 begin, Iter2 end) {
        int32_t result{0u};
        bool_t find{False};

        // // 首先尝试使用线程局部池
        // for (auto& it : thread_pool) {
        //     if (it.is_used.compare_exchange_strong(find, True)) {
        //         find = True;
        //         it.layers.clear();
        //         it.layers.reserve(_capacity);
        //         try {
        //             result = _call(json, begin, end, it.layers);
        //         } catch (...) {
        //             it.is_used = False;
        //             throw;
        //         }
        //         it.is_used = False;
        //         break;
        //     }
        // }

        // // 如果线程局部池满，则使用全局池
        // if (!find) {
        //     for (auto& it : pool) {
        //         if (it.is_used.compare_exchange_strong(find, True)) {
        //             find = True;
        //             it.layers.clear();
        //             it.layers.reserve(_capacity);
        //             try {
        //                 result = _call(json, begin, end, it.layers);
        //             } catch (...) {
        //                 it.is_used = False;
        //                 throw;
        //             }
        //             it.is_used = False;
        //             break;
        //         }
        //     }
        // }

        // 最后回退到栈分配
        if (!find) {
            std::vector<impl_type> layers;
            layers.reserve(_capacity);
            result = _call(json, begin, end, layers);
        }
        return result;
    }
};

template <class Iter1, class Iter2, class Char, memory_policy_t Policy>
constexpr inline int32_t parse(value<Char, Policy>* json, Iter1 begin, Iter2 end) noexcept {
    parser<Char, Policy> parser;
    return parser(json, begin, end);
}

#ifdef _GLIBCXX_FSTREAM
template <class Char, memory_policy_t Policy>
constexpr inline int32_t parse(value<Char, Policy>* ptr,
                               std::basic_ifstream<Char> const& stream) noexcept {
    int32_t result{0};

    do {
        if (unlikely(!stream.is_open())) {
            result = static_cast<int32_t>(error::file_not_found);
            break;
        }

        auto text =
            std::string{std::istreambuf_iterator<Char>(stream), std::istreambuf_iterator<Char>()};
        result = parse(ptr, text.data(), text.data() + text.size());
    } while (false);

    return result;
}

template <class Char, memory_policy_t Policy>
int32_t parse(value<Char, Policy>* ptr, string_view_t<Char> file) noexcept {
    return parse(ptr, std::basic_ifstream<Char>{file});
}
#endif

template <class OutStream, class Char, memory_policy_t Policy>
OutStream& operator<<(OutStream& out, value<Char, Policy> const& value) {
    value.to(out);
    return out;
}

};  // namespace json

using json_t = json::value<char, memory_policy_t::copy>;
using json_view_t = json::value<char, memory_policy_t::view>;

};  // namespace qlib
