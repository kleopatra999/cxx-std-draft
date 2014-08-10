#ifndef ALLOCATOR_GUIDE_EXAMPLE_OFFSET_POINTER
#define ALLOCATOR_GUIDE_EXAMPLE_OFFSET_POINTER

#include <cstdint>
#include <iterator>
#include <memory>
#include <type_traits>

#ifndef UINTPTR_MAX
static_assert(false, "Type uintptr_t is not available. You cannot use offset pointers on your platform.");
#endif

template <typename T>
class OffPtrBase
{
protected:
    std::uintptr_t offset;
    static std::uintptr_t const null_value = static_cast<std::uintptr_t>(-1);

    static T * add_offset(void const * p, std::uintptr_t n) noexcept
    {
        return static_cast<T *>(reinterpret_cast<void *>(reinterpret_cast<std::uintptr_t>(p) + n));
    }

    static std::uintptr_t get_offset(void const * from, void const * to) noexcept
    {
        return reinterpret_cast<std::uintptr_t>(to) - reinterpret_cast<std::uintptr_t>(from);
    }

    std::uintptr_t crement(std::ptrdiff_t n) const noexcept
    {
        return get_offset(this, get() + n);
    }

public:
    bool null() const noexcept
    {
        return offset == null_value;
    }

    T * get() const noexcept
    {
        return null() ? nullptr : add_offset(this, offset);
    }

    // Constructors
    OffPtrBase() noexcept
    : offset(null_value)
    { }

    OffPtrBase(T * native) noexcept
    : offset(native == nullptr ? null_value : get_offset(this, native))
    { }

    OffPtrBase(OffPtrBase const & rhs) noexcept
    : offset(rhs.null() ? null_value : get_offset(this, rhs.get()))
    { }

    template <typename U,
              typename = typename std::enable_if<!std::is_same<T, U>::value && std::is_same<typename std::remove_cv<T>::type, U>::value>::type>
    OffPtrBase(OffPtrBase<U> const & rhs) noexcept
    : offset(rhs.null() ? null_value : get_offset(this, rhs.get()))
    { }

    template <typename U,
              typename Dummy = void,
              typename = typename std::enable_if<!std::is_same<typename std::remove_cv<T>::type, typename std::remove_cv<U>::type>::value && !std::is_void<U>::value,
                                                 decltype(static_cast<T *>(std::declval<U *>()))>::type>
    OffPtrBase(OffPtrBase<U> const & rhs) noexcept
    : offset(rhs.null() ? null_value : get_offset(this, static_cast<T *>(rhs.get())))
    { }

    // NullablePointer requirements
    explicit operator bool() const noexcept
    {
        return !null();
    }

    OffPtrBase & operator=(OffPtrBase const & rhs) noexcept
    {
        if (this != &rhs)
        {
            offset = rhs.null() ? null_value : get_offset(this, rhs.get());
        }
        return *this;
    }

    OffPtrBase & operator=(std::nullptr_t) noexcept
    {
        offset = null_value;
        return *this;
    }
};

template <typename T> struct OffPtr;

template <>
struct OffPtr<void> : OffPtrBase<void>
{
    OffPtr() = default;
    using OffPtrBase<void>::OffPtrBase;
    using OffPtrBase<void>::operator=;
};

template <>
struct OffPtr<void const> : OffPtrBase<void const>
{
    OffPtr() = default;
    using OffPtrBase<void const>::OffPtrBase;
    using OffPtrBase<void const>::operator=;
};


template <typename T>
struct OffPtr : OffPtrBase<T>
{
    OffPtr() = default;
    using OffPtrBase<T>::OffPtrBase;
    using OffPtrBase<T>::operator=;

    explicit OffPtr(OffPtr<void> const & rhs) noexcept
    : OffPtrBase<T>(rhs.null() ? nullptr : static_cast<T *>(rhs.get()))
    { }

    explicit OffPtr(OffPtr<const void> const & rhs) noexcept
    : OffPtrBase<T>(rhs.null() ? nullptr : static_cast<T const *>(rhs.get()))
    { }

    operator OffPtr<void>() const noexcept
    {
        return { this->get() };
    }

    operator OffPtr<void const>() const noexcept
    {
        return { this->get() };
    }

    // For pointer traits
    static OffPtr pointer_to(T & x) { return OffPtr(std::addressof(x)); }

    // Random access iterator requirements (members)
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using reference = T &;
    using pointer = OffPtr<T>;

    OffPtr operator+(std::ptrdiff_t n) const
    {
        return OffPtr(this->get() + n);
    }

    OffPtr & operator+=(std::ptrdiff_t n)
    {
        this->offset = this->crement(n);
        return *this;
    }

    OffPtr operator-(std::ptrdiff_t n) const
    {
        return OffPtr(this->get() - n);
    }

    OffPtr & operator-=(std::ptrdiff_t n)
    {
        this->offset = this->crement(-n);
        return *this;
    }

    std::ptrdiff_t operator-(OffPtr const & rhs) const
    {
        return std::distance(rhs.get(), this->get());
    }

    OffPtr & operator++()
    {
        this->offset = this->crement(1);
        return *this;
    }

    OffPtr & operator--()
    {
        this->offset = this->crement(-1);
        return *this;
    }

    OffPtr operator++(int)
    {
        OffPtr tmp(*this);
        ++*this;
        return tmp;
    }

    OffPtr operator--(int)
    {
        OffPtr tmp(*this);
        --*this;
        return tmp;
    }

    T * operator->() const noexcept { return this->get(); }
    T & operator*() const noexcept { return *this->get(); }
    T & operator[](std::size_t i) const noexcept { return this->get()[i]; }
};

// Random access iterator requirements (non-members)
#define DEFINE_OPERATOR(oper, op, expr)                                                              \
  template <typename T>                                                                              \
  bool oper (OffPtr<T> const & lhs, OffPtr<T> const & rhs) noexcept                                  \
  { return expr; }                                                                                   \
  template <typename T>                                                                              \
  bool oper (OffPtr<T> const & lhs, typename std::common_type<OffPtr<T>>::type const & rhs) noexcept \
  { return lhs op rhs; }                                                                             \
  template <typename T>                                                                              \
  bool oper (typename std::common_type<OffPtr<T>>::type const & lhs, OffPtr<T> const & rhs) noexcept \
  { return lhs op rhs; }
DEFINE_OPERATOR(operator==, ==, (lhs.get() == rhs.get()))
DEFINE_OPERATOR(operator!=, !=, (lhs.get() != rhs.get()))
DEFINE_OPERATOR(operator<,  <,  (lhs.get() <  rhs.get()))
DEFINE_OPERATOR(operator<=, <=, (lhs.get() <= rhs.get()))
DEFINE_OPERATOR(operator>,  >,  (lhs.get() >  rhs.get()))
DEFINE_OPERATOR(operator>=, >=, (lhs.get() >= rhs.get()))
#undef DEFINE_OPERATOR

#endif
