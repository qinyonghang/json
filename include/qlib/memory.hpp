#pragma once

#define MEMORY_IMPLEMENTATION

// #include <iostream>
#include <new>
#include <type_traits>

#include "qlib/object.h"

namespace qlib {

namespace memory {

constexpr bool_t is_64bit = (sizeof(void*) == 8);

template <class T1, class T2>
NODISCARD INLINE static constexpr auto align_up(T1 size, T2 alignment) noexcept {
    return (size + (alignment - 1)) & ~(alignment - 1);
}

class bad_alloc final : public exception {
public:
    char const* what() const noexcept override { return "bad alloc"; }
};

// static inline uint64_t _new_count{0u};

class new_allocator : public object {
public:
    using reference = new_allocator;
    // struct traits_type final {
    //     using reference = new_allocator;
    // };

    template <class T>
    NODISCARD INLINE static constexpr T* allocate(uint64_t n) {
        static_assert(sizeof(T) > 0, "cannot allocate zero-sized object");
        // if (unlikely(!n)) {
        //     return nullptr;
        // }
        // ++_new_count;
        return (T*)(::operator new(n * sizeof(T)));
    }

    template <class T>
    INLINE static constexpr void deallocate(T* p, uint64_t n __attribute__((__unused__))) {
        // if (likely(p)) {
        ::operator delete(p
#if __cpp_sized_deallocation
                          ,
                          n * sizeof(T)
#endif
        );
        // }
    }

    template <class T, class... Args>
    INLINE static constexpr void construct(T* p, Args&&... args) noexcept(
        std::is_nothrow_constructible<T, Args...>::value) {
        new (p) T(std::forward<Args>(args)...);
    }

    template <class T>
    INLINE static constexpr void destroy(T* p) noexcept(std::is_nothrow_destructible<T>::value) {
        p->~T();
    }

    NODISCARD INLINE static constexpr uint64_t max_size() noexcept { return (uint64_t)(-1); }
};

constexpr uint64_t default_pool_size = 64 * 1024;

// static inline uint64_t _pool_count{0u};
class pool_allocator : public new_allocator {
public:
    using base = new_allocator;
    using self = pool_allocator;
    using size_type = uint64_t;

protected:
    struct alignas(8) node {
        node* next{nullptr};
        uint8_t* data{nullptr};
        // node* prev{nullptr};
        size_type used{0u};
        size_type capacity{0u};
    };

    node* _list{nullptr};

public:
    class reference : public object {
    protected:
        pool_allocator* _impl;

    public:
        INLINE constexpr reference() noexcept = delete;

        INLINE constexpr reference(pool_allocator& allocator) : _impl(&allocator) {}

        INLINE constexpr reference(reference const& other) noexcept = default;
        INLINE constexpr reference(reference&& other) noexcept = default;
        INLINE constexpr reference& operator=(reference const& other) noexcept = default;
        INLINE constexpr reference& operator=(reference&& other) noexcept = default;

        INLINE constexpr operator pool_allocator&() noexcept { return *_impl; }
        // INLINE constexpr operator pool_allocator&() noexcept { return *_impl; }
    };

    // struct traits_type final {};

    INLINE pool_allocator(size_type capacity = 64 * 1024) noexcept {
        _list = (node*)::operator new(sizeof(node) + capacity);
        _list->next = nullptr;
        _list->data = (uint8_t*)_list + sizeof(node);
        // _list->prev = _list;
        _list->used = 0u;
        _list->capacity = capacity;
        // std::cout << "sizeof(node): " << sizeof(node) << std::endl;
    }

    INLINE ~pool_allocator() noexcept {
        node* cur = _list;
        while (cur != nullptr) {
            node* next = cur->next;
            ::operator delete(cur
#if __cpp_sized_deallocation
                              ,
                              sizeof(node) + cur->capacity
#endif
            );
            cur = next;
        }
    }

    template <class T>
    NODISCARD INLINE constexpr T* allocate(size_type n) {
        static_assert(sizeof(T) > 0, "cannot allocate zero-sized object");

        size_type size = align_up(sizeof(T) * n, sizeof(void*));

        // if (unlikely(!n)) {
        //     return nullptr;
        // }

        if (unlikely(_list->used + size > _list->capacity)) {
            // std::cout << "pool allocator exhausted" << std::endl;
            // ++_pool_count;
            size_type new_capacity = _list->capacity * 2;
            while (new_capacity < size) {
                new_capacity *= 2;
            }
            auto new_node = (node*)(::operator new(sizeof(node) + new_capacity));
            new_node->next = _list;
            new_node->data = (uint8_t*)new_node + sizeof(node);
            new_node->used = 0u;
            new_node->capacity = new_capacity;
            _list = new_node;
        }
        auto ptr = (T*)(_list->data + _list->used);
        _list->used += size;
        return ptr;
    }

    template <class T>
    INLINE constexpr void deallocate(T* p __attribute__((__unused__)),
                                     uint64_t n __attribute__((__unused__))) noexcept {}
};

};  // namespace memory

using new_allocator_t = memory::new_allocator;
// template <uint64_t N = memory::default_pool_size>
using pool_allocator_t = memory::pool_allocator;

};  // namespace qlib
