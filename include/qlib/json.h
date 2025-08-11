#pragma once

#define JSON_IMPLEMENTATION

#include <array>
#include <atomic>
#include <vector>

#include "qlib/string.h"

namespace qlib {
namespace json {

template <class T>
constexpr static inline bool_t is_number_v = is_one_of_v<T,
                                                         int8_t,
                                                         int16_t,
                                                         int32_t,
                                                         int64_t,
                                                         uint8_t,
                                                         uint16_t,
                                                         uint32_t,
                                                         uint64_t,
                                                         float32_t,
                                                         float64_t,
                                                         bool_t>;

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
};

enum class value_enum : uint8_t {
    null = 0,
    object = 1 << 0,
    array = 1 << 1,
    string = 1 << 2,
    number = 1 << 3,
    number_ref = 1 << 4,
};

class not_object final : public std::exception {
public:
    char const* what() const noexcept override { return "not object"; }
};

class not_array final : public std::exception {
public:
    char const* what() const noexcept override { return "not array"; }
};

class not_string final : public std::exception {
public:
    char const* what() const noexcept override { return "not string"; }
};

class not_number final : public std::exception {
public:
    char const* what() const noexcept override { return "not number"; }
};

class no_key final : public std::exception {
public:
    char const* what() const noexcept override { return "no key"; }
};

class not_get final : public std::exception {
public:
    char const* what() const noexcept override { return "not get"; }
};

class bad_convert final : public std::exception {
public:
    char const* what() const noexcept override { return "bad convert"; }
};

template <class Char, memory_policy_t Policy>
class parser;

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
    using string_view_t = string::value<Char, memory_policy_t::view>;
    using string_t = string::value<Char, memory_policy_t::copy>;

    constexpr static inline string_view_t true_str{"true"};
    constexpr static inline string_view_t false_str{"false"};

    static string_view_t encode(bool_t value) { return value ? true_str : false_str; }

    static bool_t decode(string_view_t s) {
        if (s == true_str) {
            return True;
        } else if (s == false_str) {
            return False;
        } else {
            throw bad_convert();
        }
    }
};

template <class Char, class T>
struct converter<Char,
                 T,
                 std::enable_if_t<is_one_of_v<T,
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
    final : public object {
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

#ifdef _GLIBCXX_FS_PATH_H
template <>
struct converter<char, std::filesystem::path> final : public object {
    static string_t encode(std::filesystem::path const& value) { return string_t{value.native()}; }

    static std::filesystem::path decode(string_view_t s) {
        return std::filesystem::path{s.begin(), s.end()};
    }
};
#endif

template <class Char, memory_policy_t Policy>
class value final : public object {
public:
    using char_type = Char;
    using base = object;
    using self = value<Char, Policy>;
    using string_view_t = string::value<Char, memory_policy_t::view>;
    using string_t = string::value<Char, memory_policy_t::copy>;
    using key_type = string::value<Char, Policy>;
    using object_type = std::vector<std::pair<key_type, self>>;
    using array_type = std::vector<self>;
    using string_type = string::value<Char, Policy>;
    using number_type = string_t;
    using size_type = size_t;
    constexpr static inline memory_policy_t memory_policy = Policy;
    const static inline self default_value = self{};

protected:
    value_enum _type{value_enum::null};
    using impl_type = storage<object_type, array_type, string_type, number_type>;
    impl_type _impl;

    friend class parser<Char, Policy>;

    struct FixedOutStream final : public object {
    protected:
        template <class T>
        using uptr = std::unique_ptr<T>;

        uptr<Char> _impl{nullptr};
        size_type _size{0u};
        size_type _capacity{0u};

    public:
        class not_enough_space final : public std::exception {
        public:
            [[nodiscard]] const char* what() const noexcept override { return "not enough space"; }
        };

        explicit FixedOutStream(size_type capacity)
                : _impl(new Char[capacity + 1u]), _size(0u), _capacity(capacity) {}

        FixedOutStream(FixedOutStream const&) = delete;
        FixedOutStream& operator=(FixedOutStream const&) = delete;
        FixedOutStream(FixedOutStream&&) = default;
        FixedOutStream& operator=(FixedOutStream&&) = default;

        FixedOutStream& operator<<(string_view_t s) {
            size_type new_size = _size + s.size();
            if (unlikely(new_size > _capacity)) {
                throw not_enough_space();
            }
            std::copy(s.begin(), s.end(), _impl.get() + _size);
            _size = new_size;
            return *this;
        }

        [[nodiscard]] bool_t operator==(string_view_t s) const {
            return _size == s.size() && std::equal(s.begin(), s.end(), _impl.get());
        }
    };

public:
    class key_value_ref final : public object {
    public:
        using self = key_value_ref;
        using key_type = string_view_t;
        using value_type = value<Char, Policy>;

    protected:
        mutable key_type _key;
        mutable value_type _value;

    public:
        key_value_ref() = delete;
        key_value_ref(self const&) = delete;
        self& operator=(self const&) = delete;

        constexpr key_value_ref(self&&) = default;
        self& operator=(self&&) = default;

        template <class Key, class... Args>
        constexpr key_value_ref(Key&& key, Args&&... args)
                : _key(std::forward<Key>(key)), _value(std::forward<Args>(args)...) {}

        std::pair<key_type, value_type> operator*() const noexcept {
            return {std::move(_key), std::move(_value)};
        }
    };

    class value_ref final : public object {
    public:
        using self = value_ref;
        using value_type = value<Char, Policy>;

    protected:
        mutable value_type _impl;

    public:
        value_ref() = delete;
        value_ref(self const&) = delete;
        self& operator=(self const&) = delete;

        constexpr value_ref(self&&) = default;
        self& operator=(self&&) = default;

        template <class... Args>
        constexpr value_ref(Args&&... args) : _impl(std::forward<Args>(args)...) {}

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

    template <class T, class Enable = std::enable_if_t<is_number_v<T>>>
    constexpr value(T value) : _type(value_enum::number) {
        new (&_impl) number_type(converter<Char, T>::encode(value));
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
                new (&_impl) number_type(*(number_type*)(&o._impl));
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
                ((number_type*)&_impl)->~number_type();
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
        new (this) value(std::forward<T>(o));
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
        if constexpr (is_one_of_v<T, string_t, string_view_t>) {
            if (likely(_type == value_enum::string)) {
                return *(string_type*)(&_impl);
            } else {
                throw not_string();
            }
        } else if constexpr (is_number_v<T>) {
            if (likely(_type == value_enum::number_ref)) {
                return converter<Char, T>::decode(*(string_type*)(&_impl));
            } else if (likely(_type == value_enum::number)) {
                return converter<Char, T>::decode(*(number_type*)(&_impl));
            } else {
                throw not_number();
            }
        } else {
            static_assert(is_number_v<T> || is_one_of_v<T, string_t, string_view_t>,
                          "Unsupported type for get()");
            throw not_number();  // 这行不会执行，但为了通过编译检查
        }
    }

    template <class T>
    [[nodiscard]] constexpr T get(T&& default_value) const {
        if (empty()) {
            return std::forward<T>(default_value);
        }
        return get<std::decay_t<T>>();
    }

    [[nodiscard]] constexpr self const& operator[](string_view_t key) const {
        auto& object = this->object();
        for (auto const& pair : object) {
            if (pair.first == key) {
                return pair.second;
            }
        }
        // throw no_key();
        return default_value;
    }

    [[nodiscard]] constexpr self& operator[](string_view_t key) {
        auto& object = this->object();
        for (auto& pair : object) {
            if (pair.first == key) {
                return pair.second;
            }
        }
        object.emplace_back(key, self());
        return object.back().second;
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

    [[nodiscard]] static self object(std::initializer_list<key_value_ref> list) {
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

    [[nodiscard]] static self array(std::initializer_list<value_ref> list) {
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
                out << "null";
                break;
            }
            case value_enum::string: {
                out << "\"" << *(string_type*)(&_impl) << "\"";
                break;
            }
            case value_enum::number: {
                out << *((number_type*)(&_impl));
                break;
            }
            case value_enum::number_ref: {
                out << *((string_type*)(&_impl));
                break;
            }
            case value_enum::array: {
                out << "[";
                auto& array = this->array();
                for (auto it = array.begin(); it != array.end();) {
                    it->to(out);
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
                    out << "\"" << it->first << "\"" << ":";
                    it->second.to(out);
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
        } catch (typename FixedOutStream::not_enough_space const& _) {
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
    using number_type = typename json_type::number_type;
    using string_type = typename json_type::string_type;
    using array_type = typename json_type::array_type;
    using object_type = typename json_type::object_type;
    using key_type = typename json_type::key_type;

protected:
    size_type _capacity{16u};

    struct impl {
        storage<object_type, array_type> values;
        bool_t is_object;
        key_type key;
    };
    using impl_type = storage<impl>;

    struct pool_entry {
        std::atomic<bool_t> is_used{False};
        std::vector<impl_type> layers{};
    };

    static inline thread_local std::array<pool_entry, 4u> thread_pool{};
    static inline std::array<pool_entry, 16u> pool{};

    static inline bool_t _init_pool = []() {
        for (auto& it : pool) {
            it.is_used = False;
            it.layers.reserve(32u);
        }
        return True;
    }();

    static auto& _is_object_from_type(impl_type& value) {
        return *(bool_t*)((uint8_t*)&value + offsetof(impl, is_object));
    }

    static auto& _key_from_type(impl_type& value) {
        return *(key_type*)((uint8_t*)&value + offsetof(impl, key));
    }

    static auto& _object_from_type(impl_type& value) {
        return *(object_type*)((uint8_t*)&value + offsetof(impl, values));
    }

    static auto& _array_from_type(impl_type& value) {
        return *(array_type*)((uint8_t*)&value + offsetof(impl, values));
    }

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

    inline auto _impl_init(impl_type& value, bool_t is_object, key_type&& key) {
        _impl_init(value, is_object);
        auto& _key = _key_from_type(value);
        new (&_key) key_type(std::move(key));
    }

    inline auto _impl_emplace(impl_type& impl, key_type&& key, json_type&& value) {
        auto& object = _object_from_type(impl);
        object.emplace_back(std::move(key), std::move(value));
    }

    inline auto _impl_emplace(impl_type& impl, json_type&& value) {
        auto& array = _array_from_type(impl);
        array.emplace_back(std::move(value));
    }

    constexpr static inline bool_t is_space(char ch) noexcept {
        return (ch == 32) | (ch == 9) | (ch == 10) | (ch == 13);
    }

    template <class Iter1, class Iter2>
    constexpr static inline bool_t _is_null(Iter1 begin, Iter2 end) noexcept {
        return std::distance(begin, end) == 4u && *begin++ == 'n' && *begin++ == 'u' &&
            *begin++ == 'l' && *begin++ == 'l';
    }

    static inline constexpr bool_t whitespace_table[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,  // \t, \n, \r
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // ' '
        // ... 其余为0
    };

    template <class Iter1, class Iter2>
    constexpr static inline Iter1 _skip_space(Iter1 begin, Iter2 end) noexcept {
        // while (begin + 3 < end) {
        //     auto b1 = whitespace_table[*begin];
        //     auto b2 = whitespace_table[*(begin + 1)];
        //     auto b3 = whitespace_table[*(begin + 2)];
        //     auto b4 = whitespace_table[*(begin + 3)];
        //     if (!b1) {
        //         break;
        //     }
        //     if (!b2) {
        //         begin += 1;
        //         break;
        //     }
        //     if (!b3) {
        //         begin += 2;
        //         break;
        //     }
        //     if (!b4) {
        //         begin += 3;
        //         break;
        //     }
        //     begin += 4;
        // }
        while (begin != end && whitespace_table[*begin]) {
            ++begin;
        }
        // while (begin != end && is_space(*begin)) {
        //     ++begin;
        // }
        return begin;
    }

    template <class Iter1, class Iter2>
    constexpr int32_t _call(json_type* json,
                            Iter1 begin,
                            Iter2 end,
                            std::vector<impl_type>& layers) {
        int32_t result{0};

        do {
            begin = _skip_space(begin, end);
            if (unlikely(begin == end || *begin != '{')) {
                result = static_cast<int32_t>(error::missing_left_brace);
                break;
            }
            ++begin;

            layers.emplace_back(impl_type{});
            _impl_init(layers.front(), True);

            Iter1 key_start{};
            Iter1 key_stop{};
            bool_t exit{False};
            while (begin != end && result == 0) {
                bool is_object{_is_object_from_type(layers.back())};
                if (is_object) {
                    begin = _skip_space(begin, end);
                    if (unlikely(begin == end || *begin != '"')) {
                        result = static_cast<int32_t>(error::missing_left_quote);
                        break;
                    }
                    ++begin;
                    key_start = begin;
                    while (begin != end && *begin != '"') {
                        ++begin;
                    }
                    if (unlikely(begin == end)) {
                        result = (int32_t)error::missing_right_brace;
                        break;
                    }
                    key_stop = begin;
                    ++begin;
                    begin = _skip_space(begin, end);
                    if (unlikely(begin == end || *begin != ':')) {
                        result = static_cast<int32_t>(error::missing_colon);
                        break;
                    }
                    ++begin;
                }

                begin = _skip_space(begin, end);
                if (unlikely(begin == end)) {
                    result = (int32_t)error::missing_right_brace;
                    break;
                }

                switch (*begin) {
                    case '"': {
                        ++begin;
                        auto value_start = begin;
                        while ((begin != end) && *begin != '"') {
                            ++begin;
                        }
                        if (unlikely(begin == end)) {
                            result = (int32_t)error::missing_right_brace;
                            break;
                        }
                        auto value_stop = begin;
                        ++begin;

                        json_type value(string_type(value_start, value_stop));
                        if (is_object) {
                            _impl_emplace(layers.back(), key_type(key_start, key_stop),
                                          std::move(value));
                        } else {
                            _impl_emplace(layers.back(), std::move(value));
                        }
                        break;
                    }
                    case '[': {
                        ++begin;
                        layers.emplace_back(impl_type{});
                        if (is_object) {
                            _impl_init(layers.back(), False, key_type(key_start, key_stop));
                        } else {
                            _impl_init(layers.back(), False);
                        }
                        continue;
                    }
                    case '{': {
                        ++begin;
                        layers.emplace_back(impl_type{});
                        if (is_object) {
                            _impl_init(layers.back(), True, key_type(key_start, key_stop));
                        } else {
                            _impl_init(layers.back(), True);
                        }
                        continue;
                    }
                    default: {
                        auto value_start = begin;
                        while (begin != end && *begin != ',' && *begin != '}' && *begin != ']' &&
                               (!is_space(*begin))) {
                            ++begin;
                        }
                        if (unlikely(begin == end)) {
                            result = (int32_t)error::missing_right_brace;
                            break;
                        }
                        auto value_stop = begin;
                        auto& last_layer = layers.back();
                        json_type value;
                        if (likely(!_is_null(value_start, value_stop))) {
                            value._type = value_enum::number_ref;
                            new (&value._impl) string_type(value_start, value_stop);
                        }
                        if (is_object) {
                            _impl_emplace(last_layer, key_type(key_start, key_stop),
                                          std::move(value));
                        } else {
                            _impl_emplace(last_layer, std::move(value));
                        }
                    }
                }

                while (begin != end) {
                    if (*begin == ',') {
                        ++begin;
                        break;
                    }
                    if (*begin == ']') {
                        auto& last_layer = layers.back();
                        auto& is_object = _is_object_from_type(last_layer);
                        if (unlikely(is_object)) {
                            result = (int32_t)error::missing_left_brace;
                            break;
                        }

                        json_type value;
                        value._type = value_enum::array;
                        auto& array = _array_from_type(last_layer);
                        new (&value._impl) array_type(std::move(array));
                        auto& __last_layer = layers[layers.size() - 2];
                        if (_is_object_from_type(__last_layer)) {
                            auto& key = _key_from_type(last_layer);
                            _impl_emplace(__last_layer, std::move(key), std::move(value));
                        } else {
                            _impl_emplace(__last_layer, std::move(value));
                        }
                        layers.pop_back();
                    }
                    if (*begin == '}') {
                        auto& last_layer = layers.back();
                        auto& is_object = _is_object_from_type(last_layer);
                        if (unlikely(!is_object)) {
                            result = (int32_t)error::missing_left_brace;
                            break;
                        }
                        if (unlikely(layers.size() == 1u)) {
                            exit = True;
                            break;
                        }

                        auto& object = _object_from_type(last_layer);
                        json_type value(std::move(object));
                        auto& __last_layer = layers[layers.size() - 2];
                        if (_is_object_from_type(__last_layer)) {
                            auto& key = _key_from_type(last_layer);
                            _impl_emplace(__last_layer, std::move(key), std::move(value));
                        } else {
                            _impl_emplace(__last_layer, std::move(value));
                        }
                        layers.pop_back();
                    }
                    ++begin;
                }
                if (unlikely(begin == end)) {
                    result = (int32_t)error::missing_right_brace;
                    break;
                }
                if (unlikely(result != 0 || exit)) {
                    break;
                }
            }

            if (unlikely(result != 0)) {
                break;
            }

            auto& object = _object_from_type(layers.front());
            *json = json_type(std::move(object));
        } while (false);

        return result;
    }

public:
    constexpr parser() noexcept = default;

    constexpr parser(size_type capacity) noexcept : _capacity(capacity) {}

    template <class Iter1, class Iter2>
    int32_t operator()(json_type* json, Iter1 begin, Iter2 end) {
        int32_t result{0u};

        // 首先尝试使用线程局部池
        bool_t find{False};
        for (auto& it : thread_pool) {
            if (it.is_used.compare_exchange_strong(find, True)) {
                find = True;
                it.layers.clear();
                it.layers.reserve(_capacity);
                try {
                    result = _call(json, begin, end, it.layers);
                } catch (...) {
                    it.is_used = False;
                    throw;
                }
                it.is_used = False;
                break;
            }
        }

        // 如果线程局部池满，则使用全局池
        if (!find) {
            for (auto& it : pool) {
                if (it.is_used.compare_exchange_strong(find, True)) {
                    find = True;
                    it.layers.clear();
                    it.layers.reserve(_capacity);
                    try {
                        result = _call(json, begin, end, it.layers);
                    } catch (...) {
                        it.is_used = False;
                        throw;
                    }
                    it.is_used = False;
                    break;
                }
            }
        }

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
constexpr inline int32_t parse(value* ptr, std::basic_ifstream<Char> const& stream) noexcept {
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
int32_t parse(value* ptr, string_view_t file) noexcept {
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
