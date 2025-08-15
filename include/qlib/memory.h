#pragma once

#define MEMORY_IMPLEMENTATION

#include <iostream>
#include <new>
#include <type_traits>

#include "qlib/object.h"

namespace qlib {

namespace memory {

constexpr bool_t is_64bit = (sizeof(void*) == 8);

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
template <uint64_t N = default_pool_size>
class pool_allocator : public new_allocator {
protected:
    struct node {
        node* next{nullptr};
    };

    struct first_node : public node {
        alignas(node*) uint8_t _impl[N];
    };

    first_node _first;
    node* _current{&_first};
    uint64_t _used{0u};
    uint64_t _capacity{N};

    static inline pool_allocator _default_allocator{};

public:
    class reference : public object {
    protected:
        pool_allocator* _impl;

    public:
        INLINE constexpr reference() noexcept : _impl(&_default_allocator) {}

        INLINE constexpr reference(pool_allocator& allocator) : _impl(&allocator) {}

        INLINE constexpr reference(reference const& other) noexcept = default;
        INLINE constexpr reference(reference&& other) noexcept = default;
        INLINE constexpr reference& operator=(reference const& other) noexcept = default;
        INLINE constexpr reference& operator=(reference&& other) noexcept = default;

        INLINE constexpr operator pool_allocator&() noexcept { return *_impl; }
    };

    // struct traits_type final {};

    INLINE ~pool_allocator() noexcept {
        node* current = _first.next;
        while (current) {
            node* next = current->next;
            ::operator delete(current);
            current = next;
        }
    }

    template <class T>
    NODISCARD INLINE constexpr T* allocate(uint64_t n) {
        static_assert(sizeof(T) > 0 && sizeof(T) <= N, "cannot allocate zero-sized object");

        uint64_t size = sizeof(T) * n;
        if constexpr (sizeof(void*) == 8) {
            size = (size + 7) & ~7;
        } else {
            size = (size + 3) & ~3;
        }

        // if (unlikely(size > N)) {
        //     std::cout << "allocate " << size << " bytes" << std::endl;
        //     throw bad_alloc();
        // }

        // if (unlikely(!n)) {
        //     return nullptr;
        // }

        if (unlikely(_used + size > _capacity)) {
            // std::cout << "pool allocator exhausted" << std::endl;
            // ++_pool_count;
            uint64_t new_capacity = _capacity * 2;
            while (new_capacity < size) {
                new_capacity *= 2;
            }
            auto new_node = (node*)(::operator new(sizeof(node) + new_capacity));
            new_node->next = nullptr;
            _current->next = new_node;
            _current = new_node;
            _used = 0;
            _capacity = new_capacity;
        }
        auto ptr = (T*)((uint8_t*)_current + sizeof(node) + _used);
        _used += size;
        return ptr;
    }

    template <class T>
    INLINE constexpr void deallocate(T* p __attribute__((__unused__)),
                                     uint64_t n __attribute__((__unused__))) noexcept {}
};

};  // namespace memory

using new_allocator_t = memory::new_allocator;
template <uint64_t N = memory::default_pool_size>
using pool_allocator_t = memory::pool_allocator<N>;

};  // namespace qlib
