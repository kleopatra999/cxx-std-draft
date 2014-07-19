#ifndef ALLOCATOR_GUIDE_EXAMPLE_VLARRAY
#define ALLOCATOR_GUIDE_EXAMPLE_VLARRAY

#include <memory>
#include <type_traits>

template <typename T, typename Alloc = std::allocator<T>>
class vlarray
{
public:
    using value_type = T;
    using allocator_type = Alloc;

private:
    using traits = std::allocator_traits<allocator_type>;
    using pointer = typename traits::pointer;

public:
    using size_type = typename traits::size_type;

private:
    struct representation : allocator_type
    {
        pointer data;
        size_type size;

        explicit representation(allocator_type const & a, size_type n)
        : allocator_type(a), size(n) {}
    };

    representation r;

    void internal_clear() noexcept
    {
        if (empty()) { return; }

        allocator_type & a = r;

        for (size_type i = 0; i != size(); ++i)
        {
            size_type k = size() - i - 1;
            traits::destroy(a, data() + k);
        }

        traits::deallocate(a, r.data, size());
    }

    pointer internal_construct(allocator_type & a, value_type const * src, size_type n)
    {
        pointer p = traits::allocate(a, n);
        size_type current = 0;
        try
        {
            for (size_type i = 0; i != n; ++i, ++current)
            {
                traits::construct(a, std::addressof(*p) + i, src[i]);
            }
            return p;
        }
        catch (...)
        {
            do { traits::destroy(a, std::addressof(*p) + current); }
            while (current --> 0);
            traits::deallocate(a, p, n);
            throw;
        }
    }
    void internal_move_assign(vlarray && rhs, std::true_type) noexcept
    {
        internal_clear();
        static_cast<allocator_type &>(r) = std::move(rhs.r);
        r.data = rhs.r.data;
        r.size = rhs.r.size;
        rhs.r.data = pointer();
        rhs.r.size = 0;
    }

    void internal_move_assign(vlarray && rhs, std::false_type)
    {
        if (static_cast<allocator_type &>(r) == static_cast<allocator_type &>(rhs.r))
        {
            internal_clear();
            r.data = rhs.r.data;
            r.size = rhs.r.size;
            rhs.r.data = pointer();
            rhs.r.size = 0;
        }
        else
        {
            pointer p = internal_construct(r, rhs.data(), rhs.size());
            internal_clear();
            r.data = p;
            r.size = rhs.size();
        }
    }

public:
    value_type       * data()       noexcept { return std::addressof(*r.data); }
    value_type const * data() const noexcept { return std::addressof(*r.data); }

    value_type       & operator[](size_type i)       noexcept { return data()[i]; }
    value_type const & operator[](size_type i) const noexcept { return data()[i]; }

    size_type size() const noexcept { return r.size; }

    bool empty() const noexcept { return r.size == 0; }

    void clear() noexcept
    {
        internal_clear();
        r.data = pointer();
        r.size = 0;
    }

    allocator_type get_allocator() const noexcept { return r; }


    // Construct new container from scratch

    constexpr vlarray() noexcept(std::is_nothrow_default_constructible<allocator_type>::value)
    : vlarray(std::allocator_arg, allocator_type()) {}

    constexpr vlarray(std::allocator_arg_t, allocator_type const & alloc) noexcept
    : r(alloc, 0)
    { r.data = pointer(); }

    explicit vlarray(size_type n)
     : vlarray(n, value_type()) {}

    explicit vlarray(size_type n, value_type const & x)
    : vlarray(std::allocator_arg, allocator_type(), n, x) {}

    vlarray(std::allocator_arg_t, allocator_type const & alloc, size_type n)
    : vlarray(std::allocator_arg, alloc, n, value_type()) {}

    vlarray(std::allocator_arg_t, allocator_type const & alloc, size_type n, value_type const & x)
    : r(alloc, n)
    {
        allocator_type & a = r;
        pointer p = traits::allocate(a, size());
        for (size_type i = 0; i != size(); ++i)
        {
            traits::construct(a, std::addressof(*p) + i, x);
        }
        r.data = p;
    }


    // Destructor

    ~vlarray()
    {
        internal_clear();
    }


    // Construct container from existing container

    vlarray(vlarray const & rhs)
    : vlarray(std::allocator_arg, traits::select_on_container_copy_construction(rhs.r), rhs) {}

    vlarray(std::allocator_arg_t, allocator_type const & alloc, vlarray const & rhs)
    : r(alloc, rhs.r.size)
    {
        r.data = internal_construct(r, rhs.data(), size());
    }

    vlarray(vlarray && rhs) noexcept
    : r(rhs.r)
    {
        rhs.r.data = pointer();
        rhs.r.size = 0;
    }

    vlarray(std::allocator_arg_t, allocator_type const & alloc, vlarray && rhs)
    : r(alloc, rhs.r.size)
    {
        allocator_type & a = r;
      
        if (a == static_cast<allocator_type &>(rhs.r))
        {
            r.data = rhs.r.data;
            rhs.r.data = pointer();
            rhs.r.size = 0;
        }
        else
        {
            r.data = internal_construct(a, rhs.data(), size());
        }
    }


    // Swap

    void swap(vlarray & rhs) noexcept
    {
        std::swap(r, rhs.r);
    }


    // Assignment

    vlarray & operator=(vlarray const & rhs)
    {
        if (this == &rhs) { return *this; }

        pointer p;
        if (traits::propagate_on_container_copy_assignment::value)
        {
            allocator_type ra = rhs.r;
            p = internal_construct(ra, rhs.data(), rhs.size());
            static_cast<allocator_type &>(r) = static_cast<allocator_type const &>(rhs.r);
        }
        else
        {
            p = internal_construct(r, rhs.data(), rhs.size());
        }

        internal_clear();
        r.data = p;
        r.size = rhs.size();

        return *this;
    }

    vlarray & operator=(vlarray && rhs) noexcept(traits::propagate_on_container_move_assignment::value)
    {
        if (this != &rhs)
        {
            internal_move_assign(std::move(rhs), typename traits::propagate_on_container_move_assignment());
        }
        return *this;
    }
};

#endif
