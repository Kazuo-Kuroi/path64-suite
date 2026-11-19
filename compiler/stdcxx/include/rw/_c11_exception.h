// C++11 exception handles and nested exceptions. Original Path64 implementation.
#ifndef _RWSTD_C11_EXCEPTION_INCLUDED
#define _RWSTD_C11_EXCEPTION_INCLUDED
#include <rw/_c11_utility.h>

extern "C" {
void* __cxa_current_primary_exception() noexcept;
void __cxa_increment_exception_refcount(void*) noexcept;
void __cxa_decrement_exception_refcount(void*) noexcept;
void __cxa_rethrow_primary_exception(void*);
}
namespace std {
terminate_handler get_terminate() noexcept;
unexpected_handler get_unexpected() noexcept;
class exception_ptr;
exception_ptr current_exception() noexcept;
[[noreturn]] void rethrow_exception(exception_ptr);
class exception_ptr {
    void* _C_exception;
    exception_ptr(void* __p, int) noexcept: _C_exception(__p) { }
    friend exception_ptr current_exception() noexcept;
    friend void rethrow_exception(exception_ptr);
public:
    constexpr exception_ptr() noexcept: _C_exception(nullptr) { }
    constexpr exception_ptr(nullptr_t) noexcept: _C_exception(nullptr) { }
    exception_ptr(const exception_ptr& __p) noexcept: _C_exception(__p._C_exception) {
        __cxa_increment_exception_refcount(_C_exception);
    }
    exception_ptr(exception_ptr&& __p) noexcept: _C_exception(__p._C_exception) {
        __p._C_exception = nullptr;
    }
    ~exception_ptr() noexcept { __cxa_decrement_exception_refcount(_C_exception); }
    exception_ptr& operator=(const exception_ptr& __p) noexcept {
        exception_ptr(__p).swap(*this); return *this;
    }
    exception_ptr& operator=(exception_ptr&& __p) noexcept {
        exception_ptr(std::move(__p)).swap(*this); return *this;
    }
    void swap(exception_ptr& __p) noexcept {
        void* __old = _C_exception; _C_exception = __p._C_exception; __p._C_exception = __old;
    }
    explicit operator bool() const noexcept { return _C_exception != nullptr; }
    friend bool operator==(const exception_ptr& __a, const exception_ptr& __b) noexcept {
        return __a._C_exception == __b._C_exception;
    }
    friend bool operator!=(const exception_ptr& __a, const exception_ptr& __b) noexcept {
        return !(__a == __b);
    }
};
inline void swap(exception_ptr& __a, exception_ptr& __b) noexcept { __a.swap(__b); }
inline exception_ptr current_exception() noexcept {
    return exception_ptr(__cxa_current_primary_exception(), 0);
}
[[noreturn]] inline void rethrow_exception(exception_ptr __p) {
    __cxa_rethrow_primary_exception(__p._C_exception);
    std::terminate();
}
template<class _E> exception_ptr make_exception_ptr(_E __e) noexcept {
#ifndef _RWSTD_NO_EXCEPTIONS
    try { throw __e; }
    catch (...) { return current_exception(); }
#else
    return exception_ptr();
#endif
}
class nested_exception {
    exception_ptr _C_nested;
public:
    nested_exception() noexcept: _C_nested(current_exception()) { }
    nested_exception(const nested_exception&) noexcept = default;
    nested_exception& operator=(const nested_exception&) noexcept = default;
    virtual ~nested_exception() noexcept = default;
    [[noreturn]] void rethrow_nested() const {
        if (!_C_nested) std::terminate();
        std::rethrow_exception(_C_nested);
    }
    exception_ptr nested_ptr() const noexcept { return _C_nested; }
};
namespace __rw_c11 {
template<class _T> struct __nested_wrapper : _T, nested_exception {
    template<class _U> explicit __nested_wrapper(_U&& __value): _T(std::forward<_U>(__value)) { }
};
template<class _T> [[noreturn]] void __throw_nested(_T&& __value, true_type) {
#ifndef _RWSTD_NO_EXCEPTIONS
    throw __nested_wrapper<typename decay<_T>::type>(std::forward<_T>(__value));
#else
    std::terminate();
#endif
}
template<class _T> [[noreturn]] void __throw_nested(_T&& __value, false_type) {
#ifndef _RWSTD_NO_EXCEPTIONS
    throw std::forward<_T>(__value);
#else
    std::terminate();
#endif
}
template<class _T> void __rethrow_nested(const _T& __value, true_type) {
    if (const nested_exception* __nested = dynamic_cast<const nested_exception*>(std::addressof(__value)))
        __nested->rethrow_nested();
}
template<class _T> void __rethrow_nested(const _T&, false_type) { }
}
template<class _T> [[noreturn]] void throw_with_nested(_T&& __value) {
    typedef typename decay<_T>::type _U;
    static_assert(is_copy_constructible<_U>::value, "throw_with_nested needs a copyable exception");
    __rw_c11::__throw_nested(std::forward<_T>(__value), integral_constant<bool,
        is_class<_U>::value && !__is_final(_U) && !is_base_of<nested_exception, _U>::value>());
}
template<class _T> void rethrow_if_nested(const _T& __value) {
    __rw_c11::__rethrow_nested(__value, integral_constant<bool,
        is_polymorphic<_T>::value && (!is_base_of<nested_exception, _T>::value ||
        is_convertible<const _T*, const nested_exception*>::value)>());
}
} // namespace std
#endif
