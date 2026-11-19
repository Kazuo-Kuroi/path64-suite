// C++11 numeric string conversions. Original Path64 implementation.
#ifndef _RWSTD_C11_STRING_CONVERSIONS_INCLUDED
#define _RWSTD_C11_STRING_CONVERSIONS_INCLUDED
#include <cerrno>
#include <cstdlib>
#include <cwchar>
#include <cstdio>
#include <limits>
#include <stdexcept>

namespace std {
namespace __rw_c11 {
struct __conversion_errno {
    int saved;
    __conversion_errno(): saved(errno) { errno = 0; }
    ~__conversion_errno() { errno = saved; }
};
template<class _Result, class _Raw, class _Char>
_Result __parse_integer(const basic_string<_Char>& __s, size_t* __idx, int __base,
                        _Raw (*__parse)(const _Char*, _Char**, int), const char* __name)
{
    __conversion_errno __error;
    _Char* __end;
    const _Char* __start = __s.c_str();
    _Raw __value = __parse(__start, &__end, __base);
    if (__end == __start) throw invalid_argument(__name);
    if (errno == ERANGE || __value < static_cast<_Raw>((numeric_limits<_Result>::min)()) ||
        __value > static_cast<_Raw>((numeric_limits<_Result>::max)()))
        throw out_of_range(__name);
    if (__idx) *__idx = static_cast<size_t>(__end - __start);
    return static_cast<_Result>(__value);
}
template<class _Result, class _Char>
_Result __parse_float(const basic_string<_Char>& __s, size_t* __idx,
                      _Result (*__parse)(const _Char*, _Char**), const char* __name)
{
    __conversion_errno __error;
    _Char* __end;
    const _Char* __start = __s.c_str();
    _Result __value = __parse(__start, &__end);
    if (__end == __start) throw invalid_argument(__name);
    if (errno == ERANGE) throw out_of_range(__name);
    if (__idx) *__idx = static_cast<size_t>(__end - __start);
    return __value;
}
template<class _Value>
string __format_number(const char* __format, _Value __value)
{
    char __small[128];
    int __length = ::snprintf(__small, sizeof __small, __format, __value);
    if (__length < 0) throw runtime_error("to_string");
    if (static_cast<size_t>(__length) < sizeof __small)
        return string(__small, static_cast<size_t>(__length));
    string __result(static_cast<size_t>(__length) + 1, '\0');
    int __written = ::snprintf(&__result[0], __result.size(), __format, __value);
    if (__written != __length) throw runtime_error("to_string");
    __result.resize(static_cast<size_t>(__length));
    return __result;
}
#ifndef _RWSTD_NO_WCHAR_T
template<class _Value>
wstring __format_number(const wchar_t* __format, _Value __value)
{
    wchar_t __small[128];
    int __length = ::swprintf(__small, 128, __format, __value);
    if (__length >= 0 && __length < 128)
        return wstring(__small, static_cast<size_t>(__length));
    // swprintf need not report the required size when the buffer is short.
    size_t __capacity = 256;
    for (;;) {
        wstring __result(__capacity, L'\0');
        __length = ::swprintf(&__result[0], __capacity, __format, __value);
        if (__length >= 0 && static_cast<size_t>(__length) < __capacity) {
            __result.resize(static_cast<size_t>(__length));
            return __result;
        }
        if (__capacity > __result.max_size() / 2) throw length_error("to_wstring");
        __capacity *= 2;
    }
}
#endif
} // namespace __rw_c11
inline int stoi(const string& __s, size_t* __idx = 0, int __base = 10) {
    return __rw_c11::__parse_integer<int>(__s, __idx, __base, ::strtol, "stoi");
}
inline long stol(const string& __s, size_t* __idx = 0, int __base = 10) {
    return __rw_c11::__parse_integer<long>(__s, __idx, __base, ::strtol, "stol");
}
inline unsigned long stoul(const string& __s, size_t* __idx = 0, int __base = 10) {
    return __rw_c11::__parse_integer<unsigned long>(__s, __idx, __base, ::strtoul, "stoul");
}
inline long long stoll(const string& __s, size_t* __idx = 0, int __base = 10) {
    return __rw_c11::__parse_integer<long long>(__s, __idx, __base, ::strtoll, "stoll");
}
inline unsigned long long stoull(const string& __s, size_t* __idx = 0, int __base = 10) {
    return __rw_c11::__parse_integer<unsigned long long>(__s, __idx, __base, ::strtoull, "stoull");
}
inline float stof(const string& __s, size_t* __idx = 0) {
    return __rw_c11::__parse_float(__s, __idx, ::strtof, "stof");
}
inline double stod(const string& __s, size_t* __idx = 0) {
    return __rw_c11::__parse_float(__s, __idx, ::strtod, "stod");
}
inline long double stold(const string& __s, size_t* __idx = 0) {
    return __rw_c11::__parse_float(__s, __idx, ::strtold, "stold");
}
#ifndef _RWSTD_NO_WCHAR_T
inline int stoi(const wstring& __s, size_t* __idx = 0, int __base = 10) {
    return __rw_c11::__parse_integer<int>(__s, __idx, __base, ::wcstol, "stoi");
}
inline long stol(const wstring& __s, size_t* __idx = 0, int __base = 10) {
    return __rw_c11::__parse_integer<long>(__s, __idx, __base, ::wcstol, "stol");
}
inline unsigned long stoul(const wstring& __s, size_t* __idx = 0, int __base = 10) {
    return __rw_c11::__parse_integer<unsigned long>(__s, __idx, __base, ::wcstoul, "stoul");
}
inline long long stoll(const wstring& __s, size_t* __idx = 0, int __base = 10) {
    return __rw_c11::__parse_integer<long long>(__s, __idx, __base, ::wcstoll, "stoll");
}
inline unsigned long long stoull(const wstring& __s, size_t* __idx = 0, int __base = 10) {
    return __rw_c11::__parse_integer<unsigned long long>(__s, __idx, __base, ::wcstoull, "stoull");
}
inline float stof(const wstring& __s, size_t* __idx = 0) {
    return __rw_c11::__parse_float(__s, __idx, ::wcstof, "stof");
}
inline double stod(const wstring& __s, size_t* __idx = 0) {
    return __rw_c11::__parse_float(__s, __idx, ::wcstod, "stod");
}
inline long double stold(const wstring& __s, size_t* __idx = 0) {
    return __rw_c11::__parse_float(__s, __idx, ::wcstold, "stold");
}
#endif
inline string to_string(int __value) { return __rw_c11::__format_number("%d", __value); }
inline string to_string(unsigned __value) { return __rw_c11::__format_number("%u", __value); }
inline string to_string(long __value) { return __rw_c11::__format_number("%ld", __value); }
inline string to_string(unsigned long __value) { return __rw_c11::__format_number("%lu", __value); }
inline string to_string(long long __value) { return __rw_c11::__format_number("%lld", __value); }
inline string to_string(unsigned long long __value) { return __rw_c11::__format_number("%llu", __value); }
inline string to_string(float __value) { return __rw_c11::__format_number("%f", __value); }
inline string to_string(double __value) { return __rw_c11::__format_number("%f", __value); }
inline string to_string(long double __value) { return __rw_c11::__format_number("%Lf", __value); }
#ifndef _RWSTD_NO_WCHAR_T
inline wstring to_wstring(int __value) { return __rw_c11::__format_number(L"%d", __value); }
inline wstring to_wstring(unsigned __value) { return __rw_c11::__format_number(L"%u", __value); }
inline wstring to_wstring(long __value) { return __rw_c11::__format_number(L"%ld", __value); }
inline wstring to_wstring(unsigned long __value) { return __rw_c11::__format_number(L"%lu", __value); }
inline wstring to_wstring(long long __value) { return __rw_c11::__format_number(L"%lld", __value); }
inline wstring to_wstring(unsigned long long __value) { return __rw_c11::__format_number(L"%llu", __value); }
inline wstring to_wstring(float __value) { return __rw_c11::__format_number(L"%f", __value); }
inline wstring to_wstring(double __value) { return __rw_c11::__format_number(L"%f", __value); }
inline wstring to_wstring(long double __value) { return __rw_c11::__format_number(L"%Lf", __value); }
#endif
} // namespace std
#endif
