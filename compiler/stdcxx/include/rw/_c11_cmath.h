// C++11 mathematical overloads. Original Path64 implementation.
// Included inside namespace std by <cmath>.
#ifndef _RWSTD_C11_CMATH_INCLUDED
#define _RWSTD_C11_CMATH_INCLUDED
using ::acosf; using ::acosl;
using ::asinf; using ::asinl;
using ::atanf; using ::atanl;
using ::atan2f; using ::atan2l;
using ::ceilf; using ::ceill;
using ::cosf; using ::cosl;
using ::coshf; using ::coshl;
using ::expf; using ::expl;
using ::fabsf; using ::fabsl;
using ::floorf; using ::floorl;
using ::fmodf; using ::fmodl;
using ::frexpf; using ::frexpl;
using ::ldexpf; using ::ldexpl;
using ::logf; using ::logl;
using ::log10f; using ::log10l;
using ::modff; using ::modfl;
using ::powf; using ::powl;
using ::sinf; using ::sinl;
using ::sinhf; using ::sinhl;
using ::sqrtf; using ::sqrtl;
using ::tanf; using ::tanl;
using ::tanhf; using ::tanhl;
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
acos(_T __x) { return std::acos(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
asin(_T __x) { return std::asin(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
atan(_T __x) { return std::atan(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
ceil(_T __x) { return std::ceil(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
cos(_T __x) { return std::cos(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
cosh(_T __x) { return std::cosh(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
exp(_T __x) { return std::exp(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
fabs(_T __x) { return std::fabs(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
floor(_T __x) { return std::floor(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
log(_T __x) { return std::log(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
log10(_T __x) { return std::log10(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
sin(_T __x) { return std::sin(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
sinh(_T __x) { return std::sinh(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
sqrt(_T __x) { return std::sqrt(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
tan(_T __x) { return std::tan(double(__x)); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
tanh(_T __x) { return std::tanh(double(__x)); }
using ::acosh; using ::acoshf; using ::acoshl;
inline float acosh(float __x) { return ::acoshf(__x); }
inline long double acosh(long double __x) { return ::acoshl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
acosh(_T __x) { return ::acosh(double(__x)); }
using ::asinh; using ::asinhf; using ::asinhl;
inline float asinh(float __x) { return ::asinhf(__x); }
inline long double asinh(long double __x) { return ::asinhl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
asinh(_T __x) { return ::asinh(double(__x)); }
using ::atanh; using ::atanhf; using ::atanhl;
inline float atanh(float __x) { return ::atanhf(__x); }
inline long double atanh(long double __x) { return ::atanhl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
atanh(_T __x) { return ::atanh(double(__x)); }
using ::cbrt; using ::cbrtf; using ::cbrtl;
inline float cbrt(float __x) { return ::cbrtf(__x); }
inline long double cbrt(long double __x) { return ::cbrtl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
cbrt(_T __x) { return ::cbrt(double(__x)); }
using ::erf; using ::erff; using ::erfl;
inline float erf(float __x) { return ::erff(__x); }
inline long double erf(long double __x) { return ::erfl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
erf(_T __x) { return ::erf(double(__x)); }
using ::erfc; using ::erfcf; using ::erfcl;
inline float erfc(float __x) { return ::erfcf(__x); }
inline long double erfc(long double __x) { return ::erfcl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
erfc(_T __x) { return ::erfc(double(__x)); }
using ::exp2; using ::exp2f; using ::exp2l;
inline float exp2(float __x) { return ::exp2f(__x); }
inline long double exp2(long double __x) { return ::exp2l(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
exp2(_T __x) { return ::exp2(double(__x)); }
using ::expm1; using ::expm1f; using ::expm1l;
inline float expm1(float __x) { return ::expm1f(__x); }
inline long double expm1(long double __x) { return ::expm1l(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
expm1(_T __x) { return ::expm1(double(__x)); }
using ::lgamma; using ::lgammaf; using ::lgammal;
inline float lgamma(float __x) { return ::lgammaf(__x); }
inline long double lgamma(long double __x) { return ::lgammal(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
lgamma(_T __x) { return ::lgamma(double(__x)); }
using ::log1p; using ::log1pf; using ::log1pl;
inline float log1p(float __x) { return ::log1pf(__x); }
inline long double log1p(long double __x) { return ::log1pl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
log1p(_T __x) { return ::log1p(double(__x)); }
using ::log2; using ::log2f; using ::log2l;
inline float log2(float __x) { return ::log2f(__x); }
inline long double log2(long double __x) { return ::log2l(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
log2(_T __x) { return ::log2(double(__x)); }
using ::logb; using ::logbf; using ::logbl;
inline float logb(float __x) { return ::logbf(__x); }
inline long double logb(long double __x) { return ::logbl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
logb(_T __x) { return ::logb(double(__x)); }
using ::nearbyint; using ::nearbyintf; using ::nearbyintl;
inline float nearbyint(float __x) { return ::nearbyintf(__x); }
inline long double nearbyint(long double __x) { return ::nearbyintl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
nearbyint(_T __x) { return ::nearbyint(double(__x)); }
using ::rint; using ::rintf; using ::rintl;
inline float rint(float __x) { return ::rintf(__x); }
inline long double rint(long double __x) { return ::rintl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
rint(_T __x) { return ::rint(double(__x)); }
using ::round; using ::roundf; using ::roundl;
inline float round(float __x) { return ::roundf(__x); }
inline long double round(long double __x) { return ::roundl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
round(_T __x) { return ::round(double(__x)); }
using ::tgamma; using ::tgammaf; using ::tgammal;
inline float tgamma(float __x) { return ::tgammaf(__x); }
inline long double tgamma(long double __x) { return ::tgammal(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
tgamma(_T __x) { return ::tgamma(double(__x)); }
using ::trunc; using ::truncf; using ::truncl;
inline float trunc(float __x) { return ::truncf(__x); }
inline long double trunc(long double __x) { return ::truncl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
trunc(_T __x) { return ::trunc(double(__x)); }
using ::ilogb; using ::ilogbf; using ::ilogbl;
inline int ilogb(float __x) { return ::ilogbf(__x); }
inline int ilogb(long double __x) { return ::ilogbl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, int>::type
ilogb(_T __x) { return ::ilogb(double(__x)); }
using ::lrint; using ::lrintf; using ::lrintl;
inline long lrint(float __x) { return ::lrintf(__x); }
inline long lrint(long double __x) { return ::lrintl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, long>::type
lrint(_T __x) { return ::lrint(double(__x)); }
using ::lround; using ::lroundf; using ::lroundl;
inline long lround(float __x) { return ::lroundf(__x); }
inline long lround(long double __x) { return ::lroundl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, long>::type
lround(_T __x) { return ::lround(double(__x)); }
using ::llrint; using ::llrintf; using ::llrintl;
inline long long llrint(float __x) { return ::llrintf(__x); }
inline long long llrint(long double __x) { return ::llrintl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, long long>::type
llrint(_T __x) { return ::llrint(double(__x)); }
using ::llround; using ::llroundf; using ::llroundl;
inline long long llround(float __x) { return ::llroundf(__x); }
inline long long llround(long double __x) { return ::llroundl(__x); }
template<class _T> inline typename enable_if<is_integral<_T>::value, long long>::type
llround(_T __x) { return ::llround(double(__x)); }
template<class _T> struct __rw_math_arg {
    typedef typename conditional<is_integral<_T>::value, double, _T>::type type;
};
template<class _T, class _U, bool = is_arithmetic<_T>::value && is_arithmetic<_U>::value>
struct __rw_math_result { };
template<class _T, class _U> struct __rw_math_result<_T, _U, true> {
    typedef typename common_type<typename __rw_math_arg<_T>::type,
                                 typename __rw_math_arg<_U>::type>::type type;
};
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value,
    typename __rw_math_result<_T, _U>::type>::type
atan2(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    return std::atan2(static_cast<_R>(__x), static_cast<_R>(__y));
}
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value,
    typename __rw_math_result<_T, _U>::type>::type
fmod(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    return std::fmod(static_cast<_R>(__x), static_cast<_R>(__y));
}
using ::copysign; using ::copysignf; using ::copysignl;
inline float copysign(float __x, float __y) { return ::copysignf(__x, __y); }
inline long double copysign(long double __x, long double __y) { return ::copysignl(__x, __y); }
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value,
    typename __rw_math_result<_T, _U>::type>::type
copysign(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    return std::copysign(static_cast<_R>(__x), static_cast<_R>(__y));
}
using ::fdim; using ::fdimf; using ::fdiml;
inline float fdim(float __x, float __y) { return ::fdimf(__x, __y); }
inline long double fdim(long double __x, long double __y) { return ::fdiml(__x, __y); }
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value,
    typename __rw_math_result<_T, _U>::type>::type
fdim(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    return std::fdim(static_cast<_R>(__x), static_cast<_R>(__y));
}
using ::fmax; using ::fmaxf; using ::fmaxl;
inline float fmax(float __x, float __y) { return ::fmaxf(__x, __y); }
inline long double fmax(long double __x, long double __y) { return ::fmaxl(__x, __y); }
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value,
    typename __rw_math_result<_T, _U>::type>::type
fmax(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    return std::fmax(static_cast<_R>(__x), static_cast<_R>(__y));
}
using ::fmin; using ::fminf; using ::fminl;
inline float fmin(float __x, float __y) { return ::fminf(__x, __y); }
inline long double fmin(long double __x, long double __y) { return ::fminl(__x, __y); }
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value,
    typename __rw_math_result<_T, _U>::type>::type
fmin(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    return std::fmin(static_cast<_R>(__x), static_cast<_R>(__y));
}
using ::hypot; using ::hypotf; using ::hypotl;
inline float hypot(float __x, float __y) { return ::hypotf(__x, __y); }
inline long double hypot(long double __x, long double __y) { return ::hypotl(__x, __y); }
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value,
    typename __rw_math_result<_T, _U>::type>::type
hypot(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    return std::hypot(static_cast<_R>(__x), static_cast<_R>(__y));
}
using ::nextafter; using ::nextafterf; using ::nextafterl;
inline float nextafter(float __x, float __y) { return ::nextafterf(__x, __y); }
inline long double nextafter(long double __x, long double __y) { return ::nextafterl(__x, __y); }
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value,
    typename __rw_math_result<_T, _U>::type>::type
nextafter(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    return std::nextafter(static_cast<_R>(__x), static_cast<_R>(__y));
}
using ::remainder; using ::remainderf; using ::remainderl;
inline float remainder(float __x, float __y) { return ::remainderf(__x, __y); }
inline long double remainder(long double __x, long double __y) { return ::remainderl(__x, __y); }
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value,
    typename __rw_math_result<_T, _U>::type>::type
remainder(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    return std::remainder(static_cast<_R>(__x), static_cast<_R>(__y));
}
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
frexp(_T __x, int* __p) { return std::frexp(double(__x), __p); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
ldexp(_T __x, int __p) { return std::ldexp(double(__x), __p); }
using ::scalbn; using ::scalbnf; using ::scalbnl;
inline float scalbn(float __x, int __p) { return ::scalbnf(__x, __p); }
inline long double scalbn(long double __x, int __p) { return ::scalbnl(__x, __p); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
scalbn(_T __x, int __p) { return std::scalbn(double(__x), __p); }
using ::scalbln; using ::scalblnf; using ::scalblnl;
inline float scalbln(float __x, long __p) { return ::scalblnf(__x, __p); }
inline long double scalbln(long double __x, long __p) { return ::scalblnl(__x, __p); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
scalbln(_T __x, long __p) { return std::scalbln(double(__x), __p); }
using ::nexttoward; using ::nexttowardf; using ::nexttowardl;
inline float nexttoward(float __x, long double __p) { return ::nexttowardf(__x, __p); }
inline long double nexttoward(long double __x, long double __p) { return ::nexttowardl(__x, __p); }
template<class _T> inline typename enable_if<is_integral<_T>::value, double>::type
nexttoward(_T __x, long double __p) { return std::nexttoward(double(__x), __p); }
using ::remquo; using ::remquof; using ::remquol;
inline float remquo(float __x, float __y, int* __p) { return ::remquof(__x, __y, __p); }
inline long double remquo(long double __x, long double __y, int* __p) { return ::remquol(__x, __y, __p); }
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value,
    typename __rw_math_result<_T, _U>::type>::type
remquo(_T __x, _U __y, int* __q) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    return std::remquo(static_cast<_R>(__x), static_cast<_R>(__y), __q);
}
using ::nan; using ::nanf; using ::nanl;
using ::fma; using ::fmaf; using ::fmal;
inline float fma(float __x, float __y, float __z) { return ::fmaf(__x, __y, __z); }
inline long double fma(long double __x, long double __y, long double __z) {
    return ::fmal(__x, __y, __z);
}
template<class _T, class _U, class _V>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value &&
    is_arithmetic<_V>::value, typename __rw_math_result<
        typename __rw_math_result<_T, _U>::type, _V>::type>::type
fma(_T __x, _U __y, _V __z) {
    typedef typename __rw_math_result<typename __rw_math_result<_T, _U>::type, _V>::type _R;
    return std::fma(static_cast<_R>(__x), static_cast<_R>(__y), static_cast<_R>(__z));
}
template<class _T>
inline typename enable_if<is_arithmetic<_T>::value, bool>::type
isinf(_T __x) {
    typedef typename __rw_math_arg<_T>::type _R;
    return __builtin_isinf(static_cast<_R>(__x)) != 0;
}
template<class _T>
inline typename enable_if<is_arithmetic<_T>::value, bool>::type
isnan(_T __x) {
    typedef typename __rw_math_arg<_T>::type _R;
    return __builtin_isnan(static_cast<_R>(__x)) != 0;
}
template<class _T>
inline typename enable_if<is_arithmetic<_T>::value, bool>::type
isfinite(_T __x) {
    typedef typename __rw_math_arg<_T>::type _R;
    return __builtin_isfinite(static_cast<_R>(__x)) != 0;
}
template<class _T>
inline typename enable_if<is_arithmetic<_T>::value, bool>::type
isnormal(_T __x) {
    typedef typename __rw_math_arg<_T>::type _R;
    return __builtin_isnormal(static_cast<_R>(__x)) != 0;
}
template<class _T>
inline typename enable_if<is_arithmetic<_T>::value, bool>::type
signbit(_T __x) {
    typedef typename __rw_math_arg<_T>::type _R;
    return __builtin_signbit(static_cast<_R>(__x)) != 0;
}
template<class _T>
inline typename enable_if<is_arithmetic<_T>::value, int>::type
fpclassify(_T __x) {
    if (std::isnan(__x)) return FP_NAN;
    if (std::isinf(__x)) return FP_INFINITE;
    if (__x == 0) return FP_ZERO;
    return std::isnormal(__x) ? FP_NORMAL : FP_SUBNORMAL;
}
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value, bool>::type
isgreater(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    const _R __a = static_cast<_R>(__x), __b = static_cast<_R>(__y);
    return !std::isnan(__a) && !std::isnan(__b) && __a > __b;
}
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value, bool>::type
isgreaterequal(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    const _R __a = static_cast<_R>(__x), __b = static_cast<_R>(__y);
    return !std::isnan(__a) && !std::isnan(__b) && __a >= __b;
}
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value, bool>::type
isless(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    const _R __a = static_cast<_R>(__x), __b = static_cast<_R>(__y);
    return !std::isnan(__a) && !std::isnan(__b) && __a < __b;
}
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value, bool>::type
islessequal(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    const _R __a = static_cast<_R>(__x), __b = static_cast<_R>(__y);
    return !std::isnan(__a) && !std::isnan(__b) && __a <= __b;
}
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value, bool>::type
islessgreater(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    const _R __a = static_cast<_R>(__x), __b = static_cast<_R>(__y);
    return !std::isnan(__a) && !std::isnan(__b) && __a != __b;
}
template<class _T, class _U>
inline typename enable_if<is_arithmetic<_T>::value && is_arithmetic<_U>::value, bool>::type
isunordered(_T __x, _U __y) {
    typedef typename __rw_math_result<_T, _U>::type _R;
    const _R __a = static_cast<_R>(__x), __b = static_cast<_R>(__y);
    return std::isnan(__a) || std::isnan(__b);
}
#endif
