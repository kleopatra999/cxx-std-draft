#ifndef ALLOCATOR_GUIDE_EXAMPLE_ARENA_ALLOCATOR
#define ALLOCATOR_GUIDE_EXAMPLE_ARENA_ALLOCATOR

#include "arena.hpp"

#include <type_traits>

template <typename T = void>
struct ArenaAllocator
{
    template <typename U> friend struct ArenaAllocator;

    using value_type = T;
    using pointer = T *;

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    explicit ArenaAllocator(Arena * a) : arena(a) {}

    template <typename U>
    ArenaAllocator(ArenaAllocator<U> const & rhs) : arena(rhs.arena) {}

    pointer allocate(std::size_t n)
    {
        return static_cast<pointer>(arena->allocate(n * sizeof(T), alignof(T)));
    }

    void deallocate(pointer p, std::size_t n)
    {
        arena->deallocate(p, n * sizeof(T));
    }

    template <typename U>
    bool operator==(ArenaAllocator<U> const & rhs) const
    {
        return arena == rhs.arena;
    }

    template <typename U>
    bool operator!=(ArenaAllocator<U> const & rhs) const
    {
        return arena != rhs.arena;
    }

private:
    Arena * arena;
};

#endif
