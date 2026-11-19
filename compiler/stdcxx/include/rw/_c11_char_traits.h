// C++11 Unicode code-unit traits. Original Path64 implementation.
// Included inside namespace std after fpos is defined.
#ifndef _RWSTD_C11_CHAR_TRAITS_INCLUDED
#define _RWSTD_C11_CHAR_TRAITS_INCLUDED

template<class _Char, class _Int>
struct __rw_c11_char_traits {
    typedef _Char char_type;
    typedef _Int int_type;
    typedef _RWSTD_STREAMOFF off_type;
    typedef _RWSTD_MBSTATE_T state_type;
    typedef fpos<state_type> pos_type;

    static void assign(char_type& __a, const char_type& __b) noexcept { __a = __b; }
    static constexpr bool eq(char_type __a, char_type __b) noexcept { return __a == __b; }
    static constexpr bool lt(char_type __a, char_type __b) noexcept { return __a < __b; }
    static int compare(const char_type* __a, const char_type* __b, _RWSTD_SIZE_T __n) {
        for (_RWSTD_SIZE_T __i = 0; __i != __n; ++__i) {
            if (lt(__a[__i], __b[__i])) return -1;
            if (lt(__b[__i], __a[__i])) return 1;
        }
        return 0;
    }
    static _RWSTD_SIZE_T length(const char_type* __s) {
        _RWSTD_SIZE_T __n = 0;
        while (!eq(__s[__n], char_type())) ++__n;
        return __n;
    }
    static const char_type* find(const char_type* __s, _RWSTD_SIZE_T __n,
                                 const char_type& __c) {
        for (_RWSTD_SIZE_T __i = 0; __i != __n; ++__i)
            if (eq(__s[__i], __c)) return __s + __i;
        return 0;
    }
    static char_type* move(char_type* __to, const char_type* __from, _RWSTD_SIZE_T __n) {
        if (__n) _RWSTD_MEMMOVE(__to, __from, __n * sizeof(char_type));
        return __to;
    }
    static char_type* copy(char_type* __to, const char_type* __from, _RWSTD_SIZE_T __n) {
        if (__n) _RWSTD_MEMCPY(__to, __from, __n * sizeof(char_type));
        return __to;
    }
    static char_type* assign(char_type* __to, _RWSTD_SIZE_T __n, char_type __c) {
        for (_RWSTD_SIZE_T __i = 0; __i != __n; ++__i) __to[__i] = __c;
        return __to;
    }
    static constexpr bool eq_int_type(int_type __a, int_type __b) noexcept { return __a == __b; }
    static constexpr int_type eof() noexcept { return static_cast<int_type>(-1); }
    static constexpr int_type not_eof(int_type __c) noexcept { return __c == eof() ? 0 : __c; }
    static constexpr char_type to_char_type(int_type __c) noexcept { return static_cast<char_type>(__c); }
    static constexpr int_type to_int_type(char_type __c) noexcept { return static_cast<int_type>(__c); }
};
template<> struct char_traits<char16_t> : __rw_c11_char_traits<char16_t, uint_least16_t> { };
template<> struct char_traits<char32_t> : __rw_c11_char_traits<char32_t, uint_least32_t> { };
#endif
