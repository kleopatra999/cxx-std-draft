#ifndef ALLOCATOR_GUIDE_EXAMPLE_OFFSET_POINTER_ALLOCATOR
#define ALLOCATOR_GUIDE_EXAMPLE_OFFSET_POINTER_ALLOCATOR

/* Example allocator that uses the fancy pointer OffPtr<T>. This particular
 * allocator is stateless and does nothing useful beyond providing a use case
 * for OffPtr<T>.
 */

#include <type_traits>
#include "offptr.hpp"

template <typename T>
struct Alloc
{
    using value_type = T;
    using pointer = OffPtr<value_type>;

    Alloc() = default;

    template <typename Other>
    Alloc(Alloc<Other> const &) noexcept
    { }

    pointer allocate(std::size_t n)
    {
        return pointer(static_cast<value_type *>(::operator new(n * sizeof(T))));
    }

    void deallocate(pointer p, std::size_t)
    {
        ::operator delete(p.get());
    }
};

template <typename T>
bool operator==(Alloc<T> const &, Alloc<T> const &) { return true; }

template <typename T>
bool operator!=(Alloc<T> const &, Alloc<T> const &) { return false; }

#endif
