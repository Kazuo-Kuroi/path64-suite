// Original Path64 C++11 numeric limits; compiler constants describe the target.
#ifndef _RWSTD_C11_LIMITS_INCLUDED
#define _RWSTD_C11_LIMITS_INCLUDED
#include <rw/_defs.h>
#include <climits>
namespace std {
enum float_round_style { round_indeterminate=-1,round_toward_zero=0,round_to_nearest=1,round_toward_infinity=2,round_toward_neg_infinity=3 };
enum float_denorm_style { denorm_indeterminate=-1,denorm_absent=0,denorm_present=1 };
namespace __rw_limits {
#ifdef _RWSTD_NO_INT_TRAPS
constexpr bool __int_traps=false;
#else
constexpr bool __int_traps=true;
#endif
#ifdef _RWSTD_NO_DBL_TRAPS
constexpr bool __float_traps=false;
#else
constexpr bool __float_traps=true;
#endif
#ifdef _RWSTD_NO_SIGNALING_NAN
constexpr bool __signaling=false;
#else
constexpr bool __signaling=true;
#endif
#ifdef _RWSTD_FLT_ROUNDS
constexpr float_round_style __rounding=float_round_style(_RWSTD_FLT_ROUNDS);
#else
constexpr float_round_style __rounding=round_indeterminate;
#endif
template<class _T> struct __base {
    static constexpr bool is_specialized=false;
    static constexpr int digits=0;
    static constexpr int digits10=0;
    static constexpr int max_digits10=0;
    static constexpr bool is_signed=false;
    static constexpr bool is_integer=false;
    static constexpr bool is_exact=false;
    static constexpr int radix=0;
    static constexpr int min_exponent=0;
    static constexpr int min_exponent10=0;
    static constexpr int max_exponent=0;
    static constexpr int max_exponent10=0;
    static constexpr bool has_infinity=false;
    static constexpr bool has_quiet_NaN=false;
    static constexpr bool has_signaling_NaN=false;
    static constexpr float_denorm_style has_denorm=denorm_absent;
    static constexpr bool has_denorm_loss=false;
    static constexpr bool is_iec559=false;
    static constexpr bool is_bounded=false;
    static constexpr bool is_modulo=false;
    static constexpr bool traps=false;
    static constexpr bool tinyness_before=false;
    static constexpr float_round_style round_style=round_toward_zero;
    static constexpr _T (min)() noexcept { return _T(); }
    static constexpr _T (max)() noexcept { return _T(); }
    static constexpr _T (lowest)() noexcept { return _T(); }
    static constexpr _T (epsilon)() noexcept { return _T(); }
    static constexpr _T (round_error)() noexcept { return _T(); }
    static constexpr _T (infinity)() noexcept { return _T(); }
    static constexpr _T (quiet_NaN)() noexcept { return _T(); }
    static constexpr _T (signaling_NaN)() noexcept { return _T(); }
    static constexpr _T (denorm_min)() noexcept { return _T(); }
};
template<class _T> constexpr bool __base<_T>::is_specialized;
template<class _T> constexpr int __base<_T>::digits;
template<class _T> constexpr int __base<_T>::digits10;
template<class _T> constexpr int __base<_T>::max_digits10;
template<class _T> constexpr bool __base<_T>::is_signed;
template<class _T> constexpr bool __base<_T>::is_integer;
template<class _T> constexpr bool __base<_T>::is_exact;
template<class _T> constexpr int __base<_T>::radix;
template<class _T> constexpr int __base<_T>::min_exponent;
template<class _T> constexpr int __base<_T>::min_exponent10;
template<class _T> constexpr int __base<_T>::max_exponent;
template<class _T> constexpr int __base<_T>::max_exponent10;
template<class _T> constexpr bool __base<_T>::has_infinity;
template<class _T> constexpr bool __base<_T>::has_quiet_NaN;
template<class _T> constexpr bool __base<_T>::has_signaling_NaN;
template<class _T> constexpr float_denorm_style __base<_T>::has_denorm;
template<class _T> constexpr bool __base<_T>::has_denorm_loss;
template<class _T> constexpr bool __base<_T>::is_iec559;
template<class _T> constexpr bool __base<_T>::is_bounded;
template<class _T> constexpr bool __base<_T>::is_modulo;
template<class _T> constexpr bool __base<_T>::traps;
template<class _T> constexpr bool __base<_T>::tinyness_before;
template<class _T> constexpr float_round_style __base<_T>::round_style;
template<class _T,_T __Min,_T __Max,bool __Bool=false> struct __integer : __base<_T> {
    static constexpr bool is_specialized=true;
    static constexpr bool is_signed=__Min<0;
    static constexpr bool is_integer=true;
    static constexpr bool is_exact=true;
    static constexpr int digits=__Bool?1:int(sizeof(_T)*CHAR_BIT)-int(is_signed);
    static constexpr int digits10=digits*301/1000;
    static constexpr int radix=2;
    static constexpr bool is_bounded=true;
    static constexpr bool is_modulo=!is_signed && !__Bool;
    static constexpr bool traps=!__Bool && __int_traps;
    static constexpr _T (min)() noexcept { return __Min; }
    static constexpr _T (max)() noexcept { return __Max; }
    static constexpr _T lowest() noexcept { return __Min; }
};
template<class _T,_T __Min,_T __Max,bool __Bool> constexpr bool __integer<_T,__Min,__Max,__Bool>::is_specialized;
template<class _T,_T __Min,_T __Max,bool __Bool> constexpr bool __integer<_T,__Min,__Max,__Bool>::is_signed;
template<class _T,_T __Min,_T __Max,bool __Bool> constexpr bool __integer<_T,__Min,__Max,__Bool>::is_integer;
template<class _T,_T __Min,_T __Max,bool __Bool> constexpr bool __integer<_T,__Min,__Max,__Bool>::is_exact;
template<class _T,_T __Min,_T __Max,bool __Bool> constexpr int __integer<_T,__Min,__Max,__Bool>::digits;
template<class _T,_T __Min,_T __Max,bool __Bool> constexpr int __integer<_T,__Min,__Max,__Bool>::digits10;
template<class _T,_T __Min,_T __Max,bool __Bool> constexpr int __integer<_T,__Min,__Max,__Bool>::radix;
template<class _T,_T __Min,_T __Max,bool __Bool> constexpr bool __integer<_T,__Min,__Max,__Bool>::is_bounded;
template<class _T,_T __Min,_T __Max,bool __Bool> constexpr bool __integer<_T,__Min,__Max,__Bool>::is_modulo;
template<class _T,_T __Min,_T __Max,bool __Bool> constexpr bool __integer<_T,__Min,__Max,__Bool>::traps;
template<class _T> struct __values;
template<> struct __values<float> {
    static constexpr float (min)() noexcept { return __FLT_MIN__; }
    static constexpr float (max)() noexcept { return __FLT_MAX__; }
    static constexpr float (epsilon)() noexcept { return __FLT_EPSILON__; }
    static constexpr float (denorm_min)() noexcept { return __FLT_DENORM_MIN__; }
    static constexpr float infinity() noexcept { return __builtin_huge_valf(); }
    static constexpr float quiet_NaN() noexcept { return __builtin_nanf(""); }
    static constexpr float signaling_NaN() noexcept { return __builtin_nansf(""); }
    static constexpr int digits() noexcept { return __FLT_MANT_DIG__; }
    static constexpr int digits10() noexcept { return __FLT_DIG__; }
    static constexpr int min_exp() noexcept { return __FLT_MIN_EXP__; }
    static constexpr int min_exp10() noexcept { return __FLT_MIN_10_EXP__; }
    static constexpr int max_exp() noexcept { return __FLT_MAX_EXP__; }
    static constexpr int max_exp10() noexcept { return __FLT_MAX_10_EXP__; }
    static constexpr int has_inf() noexcept { return __FLT_HAS_INFINITY__; }
    static constexpr int has_nan() noexcept { return __FLT_HAS_QUIET_NAN__; }
    static constexpr int denorm() noexcept { return __FLT_HAS_DENORM__; }
};
template<> struct __values<double> {
    static constexpr double (min)() noexcept { return __DBL_MIN__; }
    static constexpr double (max)() noexcept { return __DBL_MAX__; }
    static constexpr double (epsilon)() noexcept { return __DBL_EPSILON__; }
    static constexpr double (denorm_min)() noexcept { return __DBL_DENORM_MIN__; }
    static constexpr double infinity() noexcept { return __builtin_huge_val(); }
    static constexpr double quiet_NaN() noexcept { return __builtin_nan(""); }
    static constexpr double signaling_NaN() noexcept { return __builtin_nans(""); }
    static constexpr int digits() noexcept { return __DBL_MANT_DIG__; }
    static constexpr int digits10() noexcept { return __DBL_DIG__; }
    static constexpr int min_exp() noexcept { return __DBL_MIN_EXP__; }
    static constexpr int min_exp10() noexcept { return __DBL_MIN_10_EXP__; }
    static constexpr int max_exp() noexcept { return __DBL_MAX_EXP__; }
    static constexpr int max_exp10() noexcept { return __DBL_MAX_10_EXP__; }
    static constexpr int has_inf() noexcept { return __DBL_HAS_INFINITY__; }
    static constexpr int has_nan() noexcept { return __DBL_HAS_QUIET_NAN__; }
    static constexpr int denorm() noexcept { return __DBL_HAS_DENORM__; }
};
template<> struct __values<long double> {
    static constexpr long double (min)() noexcept { return __LDBL_MIN__; }
    static constexpr long double (max)() noexcept { return __LDBL_MAX__; }
    static constexpr long double (epsilon)() noexcept { return __LDBL_EPSILON__; }
    static constexpr long double (denorm_min)() noexcept { return __LDBL_DENORM_MIN__; }
    static constexpr long double infinity() noexcept { return __builtin_huge_vall(); }
    static constexpr long double quiet_NaN() noexcept { return __builtin_nanl(""); }
    static constexpr long double signaling_NaN() noexcept { return __builtin_nansl(""); }
    static constexpr int digits() noexcept { return __LDBL_MANT_DIG__; }
    static constexpr int digits10() noexcept { return __LDBL_DIG__; }
    static constexpr int min_exp() noexcept { return __LDBL_MIN_EXP__; }
    static constexpr int min_exp10() noexcept { return __LDBL_MIN_10_EXP__; }
    static constexpr int max_exp() noexcept { return __LDBL_MAX_EXP__; }
    static constexpr int max_exp10() noexcept { return __LDBL_MAX_10_EXP__; }
    static constexpr int has_inf() noexcept { return __LDBL_HAS_INFINITY__; }
    static constexpr int has_nan() noexcept { return __LDBL_HAS_QUIET_NAN__; }
    static constexpr int denorm() noexcept { return __LDBL_HAS_DENORM__; }
};
template<class _T> struct __floating : __base<_T> {
    static constexpr bool is_specialized=true;
    static constexpr bool is_signed=true;
    static constexpr int digits=__values<_T>::digits();
    static constexpr int digits10=__values<_T>::digits10();
    static constexpr int max_digits10=__FLT_RADIX__==10?digits:2+digits*30103/100000;
    static constexpr int radix=__FLT_RADIX__;
    static constexpr int min_exponent=__values<_T>::min_exp();
    static constexpr int min_exponent10=__values<_T>::min_exp10();
    static constexpr int max_exponent=__values<_T>::max_exp();
    static constexpr int max_exponent10=__values<_T>::max_exp10();
    static constexpr bool has_infinity=__values<_T>::has_inf();
    static constexpr bool has_quiet_NaN=__values<_T>::has_nan();
    static constexpr bool has_signaling_NaN=has_quiet_NaN && __signaling;
    static constexpr float_denorm_style has_denorm=float_denorm_style(__values<_T>::denorm());
    static constexpr bool is_iec559=has_infinity && has_quiet_NaN && has_denorm==denorm_present && __FLT_RADIX__==2 && (digits==24 || digits==53 || digits==64 || digits==113);
    static constexpr bool is_bounded=true;
    static constexpr bool traps=__float_traps;
    static constexpr float_round_style round_style=__rounding;
    static constexpr _T (min)() noexcept { return __values<_T>::min(); }
    static constexpr _T (max)() noexcept { return __values<_T>::max(); }
    static constexpr _T (epsilon)() noexcept { return __values<_T>::epsilon(); }
    static constexpr _T lowest() noexcept { return -__values<_T>::max(); }
    static constexpr _T round_error() noexcept { return round_style==round_to_nearest?_T(0.5):_T(1); }
    static constexpr _T infinity() noexcept { return has_infinity?__values<_T>::infinity():_T(0); }
    static constexpr _T quiet_NaN() noexcept { return has_quiet_NaN?__values<_T>::quiet_NaN():_T(0); }
    static constexpr _T signaling_NaN() noexcept { return has_signaling_NaN?__values<_T>::signaling_NaN():_T(0); }
    static constexpr _T denorm_min() noexcept { return has_denorm==denorm_present?__values<_T>::denorm_min():_T(0); }
};
template<class _T> constexpr bool __floating<_T>::is_specialized;
template<class _T> constexpr bool __floating<_T>::is_signed;
template<class _T> constexpr int __floating<_T>::digits;
template<class _T> constexpr int __floating<_T>::digits10;
template<class _T> constexpr int __floating<_T>::max_digits10;
template<class _T> constexpr int __floating<_T>::radix;
template<class _T> constexpr int __floating<_T>::min_exponent;
template<class _T> constexpr int __floating<_T>::min_exponent10;
template<class _T> constexpr int __floating<_T>::max_exponent;
template<class _T> constexpr int __floating<_T>::max_exponent10;
template<class _T> constexpr bool __floating<_T>::has_infinity;
template<class _T> constexpr bool __floating<_T>::has_quiet_NaN;
template<class _T> constexpr bool __floating<_T>::has_signaling_NaN;
template<class _T> constexpr float_denorm_style __floating<_T>::has_denorm;
template<class _T> constexpr bool __floating<_T>::is_iec559;
template<class _T> constexpr bool __floating<_T>::is_bounded;
template<class _T> constexpr bool __floating<_T>::traps;
template<class _T> constexpr float_round_style __floating<_T>::round_style;
}
template<class _T> struct numeric_limits : __rw_limits::__base<_T> {};
template<> struct numeric_limits<bool> : __rw_limits::__integer<bool,(false),(true),true> {};
#ifdef __CHAR_UNSIGNED__
template<> struct numeric_limits<char> : __rw_limits::__integer<char,0,char(static_cast<unsigned char>(-1))> {};
#else
template<> struct numeric_limits<char> : __rw_limits::__integer<char,(-__SCHAR_MAX__-1),__SCHAR_MAX__> {};
#endif
template<> struct numeric_limits<signed char> : __rw_limits::__integer<signed char,(SCHAR_MIN),(SCHAR_MAX)> {};
template<> struct numeric_limits<unsigned char> : __rw_limits::__integer<unsigned char,(0),(UCHAR_MAX)> {};
template<> struct numeric_limits<short> : __rw_limits::__integer<short,(SHRT_MIN),(SHRT_MAX)> {};
template<> struct numeric_limits<unsigned short> : __rw_limits::__integer<unsigned short,(0),(USHRT_MAX)> {};
template<> struct numeric_limits<int> : __rw_limits::__integer<int,(INT_MIN),(INT_MAX)> {};
template<> struct numeric_limits<unsigned int> : __rw_limits::__integer<unsigned int,(0),(UINT_MAX)> {};
template<> struct numeric_limits<long> : __rw_limits::__integer<long,(LONG_MIN),(LONG_MAX)> {};
template<> struct numeric_limits<unsigned long> : __rw_limits::__integer<unsigned long,(0),(ULONG_MAX)> {};
template<> struct numeric_limits<long long> : __rw_limits::__integer<long long,(LLONG_MIN),(LLONG_MAX)> {};
template<> struct numeric_limits<unsigned long long> : __rw_limits::__integer<unsigned long long,(0),(ULLONG_MAX)> {};
template<> struct numeric_limits<wchar_t> : __rw_limits::__integer<wchar_t,((__WCHAR_MAX__>0 && wchar_t(-1)>0)?0:-__WCHAR_MAX__-1),(__WCHAR_MAX__)> {};
template<> struct numeric_limits<char16_t> : __rw_limits::__integer<char16_t,(0),(char16_t(-1))> {};
template<> struct numeric_limits<char32_t> : __rw_limits::__integer<char32_t,(0),(char32_t(-1))> {};
#ifdef __SIZEOF_INT128__
template<> struct numeric_limits<unsigned __int128> : __rw_limits::__integer<unsigned __int128,0,static_cast<unsigned __int128>(-1)> {};
template<> struct numeric_limits<__int128> : __rw_limits::__integer<__int128,-static_cast<__int128>(static_cast<unsigned __int128>(-1)>>1)-1,static_cast<__int128>(static_cast<unsigned __int128>(-1)>>1)> {};
#endif
template<> struct numeric_limits<float> : __rw_limits::__floating<float> {};
template<> struct numeric_limits<double> : __rw_limits::__floating<double> {};
template<> struct numeric_limits<long double> : __rw_limits::__floating<long double> {};
template<class _T> struct numeric_limits<const _T> : numeric_limits<_T> {};
template<class _T> struct numeric_limits<volatile _T> : numeric_limits<_T> {};
template<class _T> struct numeric_limits<const volatile _T> : numeric_limits<_T> {};
}
#endif
