// Original C++11 numeric-limits constant-expression and ODR-use checks.
// UNSUPPORTED: c++98, c++03
#include <limits>
#include <climits>
#include <cfloat>
#include <cmath>
#include <type_traits>
#include <cassert>
#include <cstring>

template<class T> void integer() {
    typedef std::numeric_limits<T> L;
    static_assert(L::is_specialized && L::is_integer && L::is_exact && L::is_bounded,"integer traits");
    static_assert(L::max_digits10==0 && L::radix==2,"integer precision");
    static_assert(L::lowest()==(L::min)(),"integer lowest");
    static_assert(L::epsilon()==0 && L::round_error()==0 && L::infinity()==0,"integer functions");
    static_assert(L::quiet_NaN()==0 && L::signaling_NaN()==0 && L::denorm_min()==0,"integer nonfinite functions");
    static_assert(noexcept(L::lowest()) && noexcept((L::min)()),"noexcept observers");
    const volatile int* digits=&L::digits; assert(*digits==L::digits);
    static_assert(std::is_same<decltype(L::lowest()),T>::value,"observer result type");
    static_assert(std::numeric_limits<const volatile T>::digits==L::digits,"cv traits");
}
template<class T> void real() {
    typedef std::numeric_limits<T> L;
    constexpr T minimum=(L::min)(),maximum=(L::max)(),lowest=L::lowest();
    constexpr T epsilon=L::epsilon(),error=L::round_error(),denorm=L::denorm_min(),inf=L::infinity(),nan=L::quiet_NaN();
    static_assert(minimum>0 && lowest==-maximum && epsilon>0 && error>0,"constexpr finite values");
    static_assert(denorm>0 && denorm<minimum && inf>maximum && nan!=nan,"constexpr nonfinite/subnormal values");
    static_assert(L::is_signed && !L::is_integer && !L::is_exact && L::is_bounded,"floating traits");
    static_assert(L::is_iec559 && !L::is_modulo && L::has_denorm==std::denorm_present,"IEEE target");
    static_assert(noexcept(L::quiet_NaN()) && noexcept(L::signaling_NaN()),"noexcept nonfinite observers");
    const volatile int* digits=&L::max_digits10; assert(*digits==L::max_digits10);
    const volatile std::float_denorm_style* style=&L::has_denorm; assert(*style==std::denorm_present);
    assert(std::isinf(inf) && std::isnan(nan));
    // The signaling NaN observer is itself a constant expression.
    constexpr T signaling=L::signaling_NaN();
    const volatile T* address=&signaling; (void)address;
    static_assert(std::is_same<decltype(L::lowest()),T>::value,"floating observer result type");
}
struct NotNumeric { int value; constexpr NotNumeric():value(0) {} };
int main() {
    integer<bool>(); integer<char>(); integer<signed char>(); integer<unsigned char>(); integer<wchar_t>();
    integer<short>(); integer<unsigned short>(); integer<int>(); integer<unsigned>(); integer<long>(); integer<unsigned long>();
    integer<long long>(); integer<unsigned long long>(); integer<char16_t>(); integer<char32_t>();
    real<float>(); real<double>(); real<long double>();
    constexpr float fs=std::numeric_limits<float>::signaling_NaN();
    constexpr double ds=std::numeric_limits<double>::signaling_NaN();
    unsigned f; unsigned long long d;
    std::memcpy(&f,&fs,sizeof f); std::memcpy(&d,&ds,sizeof d);
    assert((f&0x7f800000U)==0x7f800000U && (f&0x00400000U)==0 && (f&0x003fffffU)!=0);
    assert((d&0x7ff0000000000000ULL)==0x7ff0000000000000ULL && (d&0x0008000000000000ULL)==0 && (d&0x0007ffffffffffffULL)!=0);
#ifdef __CHAR_UNSIGNED__
    static_assert(!std::numeric_limits<char>::is_signed && (std::numeric_limits<char>::min)()==0 &&
                  (std::numeric_limits<char>::max)()==UCHAR_MAX,"unsigned plain char mode");
#else
    static_assert(std::numeric_limits<char>::is_signed && (std::numeric_limits<char>::min)()==SCHAR_MIN &&
                  (std::numeric_limits<char>::max)()==SCHAR_MAX,"signed plain char mode");
#endif
    constexpr long double widened_inf=static_cast<long double>(std::numeric_limits<float>::infinity());
    constexpr float narrowed_nan=static_cast<float>(std::numeric_limits<double>::quiet_NaN());
    static_assert(widened_inf>(std::numeric_limits<long double>::max)() && narrowed_nan!=narrowed_nan,"constant format conversions");
    static_assert(std::numeric_limits<char16_t>::digits==16 && std::numeric_limits<char32_t>::digits==32,"Unicode types");
    static_assert(std::numeric_limits<float>::max_digits10==9 && std::numeric_limits<double>::max_digits10==17,"binary target round trips");
    static_assert((std::numeric_limits<int>::min)()==INT_MIN && (std::numeric_limits<unsigned>::max)()==UINT_MAX,"integer C constants");
    static_assert((std::numeric_limits<float>::min)()==FLT_MIN && (std::numeric_limits<double>::max)()==DBL_MAX,"floating C constants");
    static_assert(std::numeric_limits<long double>::digits==LDBL_MANT_DIG,"target long double");
    static_assert(!std::numeric_limits<NotNumeric>::is_specialized && std::numeric_limits<NotNumeric>::max_digits10==0,"primary template");
    static_assert(std::numeric_limits<NotNumeric>::lowest().value==0,"primary functions");
#ifdef __SIZEOF_INT128__
    integer<__int128>(); integer<unsigned __int128>();
    static_assert(std::numeric_limits<__int128>::digits==127 && std::numeric_limits<unsigned __int128>::digits==128,"extension widths");
#endif
}
