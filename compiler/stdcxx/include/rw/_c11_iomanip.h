// C++11 locale manipulators. Original Path64 implementation.
#ifndef _RWSTD_C11_IOMANIP_INCLUDED
#define _RWSTD_C11_IOMANIP_INCLUDED
#include <istream>
#include <ostream>
#include <locale>
#include <ctime>

namespace std {
namespace __rw_c11 {
template<class _Action> struct __input_manip { _Action action; };
template<class _Action> struct __output_manip { _Action action; };
template<class _Money> struct __money_reader {
    _Money& value;
    bool international;
    template<class _C, class _T>
    void apply(basic_istream<_C, _T>& __s, ios_base::iostate& __state) const {
        typedef istreambuf_iterator<_C, _T> _It;
        use_facet<money_get<_C, _It> >(__s.getloc()).get(
            _It(__s), _It(), international, __s, __state, value);
    }
};
template<class _Money> struct __money_writer {
    const _Money& value;
    bool international;
    template<class _C, class _T>
    bool apply(basic_ostream<_C, _T>& __s) const {
        typedef ostreambuf_iterator<_C, _T> _It;
        return !use_facet<money_put<_C, _It> >(__s.getloc()).put(
            _It(__s), international, __s, __s.fill(), value).failed();
    }
};
template<class _C> struct __time_reader {
    tm* value;
    const _C* format;
    template<class _T>
    void apply(basic_istream<_C, _T>& __s, ios_base::iostate& __state) const {
        typedef istreambuf_iterator<_C, _T> _It;
        use_facet<time_get<_C, _It> >(__s.getloc()).get(_It(__s), _It(), __s,
            __state, value, format, format + char_traits<_C>::length(format));
    }
};
template<class _C> struct __time_writer {
    const tm* value;
    const _C* format;
    template<class _T>
    bool apply(basic_ostream<_C, _T>& __s) const {
        typedef ostreambuf_iterator<_C, _T> _It;
        return !use_facet<time_put<_C, _It> >(__s.getloc()).put(_It(__s), __s,
            __s.fill(), value, format, format + char_traits<_C>::length(format)).failed();
    }
};
// Preserve the original facet exception when badbit is in the exception mask.
template<class _Stream> void __manip_badbit(_Stream& __s) {
    __s.setstate(static_cast<ios_base::iostate>(ios_base::badbit | _RWSTD_IOS_NOTHROW));
}
}
template<class _C, class _T, class _Action>
basic_istream<_C, _T>& operator>>(basic_istream<_C, _T>& __s,
                                  const __rw_c11::__input_manip<_Action>& __m) {
    ios_base::iostate __state = ios_base::goodbit;
    _TRY {
        typename basic_istream<_C, _T>::sentry __guard(__s);
        if (__guard) __m.action.apply(__s, __state);
    } _CATCH(...) {
        __rw_c11::__manip_badbit(__s);
        if (__s.exceptions() & ios_base::badbit) _RETHROW;
    }
    if (__state != ios_base::goodbit) __s.setstate(__state);
    return __s;
}
template<class _C, class _T, class _Action>
basic_ostream<_C, _T>& operator<<(basic_ostream<_C, _T>& __s,
                                  const __rw_c11::__output_manip<_Action>& __m) {
    bool __failed = false;
    _TRY {
        typename basic_ostream<_C, _T>::sentry __guard(__s);
        if (__guard) __failed = !__m.action.apply(__s);
    } _CATCH(...) {
        __rw_c11::__manip_badbit(__s);
        if (__s.exceptions() & ios_base::badbit) _RETHROW;
    }
    if (__failed) __s.setstate(ios_base::badbit);
    return __s;
}
template<class _Money>
inline __rw_c11::__input_manip<__rw_c11::__money_reader<_Money> >
get_money(_Money& __value, bool __intl = false) { return {{__value, __intl}}; }
template<class _Money>
inline __rw_c11::__output_manip<__rw_c11::__money_writer<_Money> >
put_money(const _Money& __value, bool __intl = false) { return {{__value, __intl}}; }
template<class _C>
inline __rw_c11::__input_manip<__rw_c11::__time_reader<_C> >
get_time(tm* __value, const _C* __format) { return {{__value, __format}}; }
template<class _C>
inline __rw_c11::__output_manip<__rw_c11::__time_writer<_C> >
put_time(const tm* __value, const _C* __format) { return {{__value, __format}}; }
}
#endif
