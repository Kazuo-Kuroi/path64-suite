// Original C++11 C-header integration and numerical edge-case regression.
// UNSUPPORTED: c++98, c++03
#include <cinttypes>
#include <cstdint>
#include <cfenv>
#include <cuchar>
#include <ctgmath>
#include <cstdbool>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <clocale>
#include <cstring>
#include <limits>
#include <type_traits>
#include <cassert>
#ifndef __bool_true_false_are_defined
#error missing C++11 boolean compatibility macro
#endif
#ifdef bool
#error bool must remain a keyword
#endif
namespace user {
struct X {}; struct Y {};
X hypot(X, Y) { return X(); }
X fma(X, Y, X) { return X(); }
}
static int calls;
static double once(double x) { ++calls; return x; }
int main() {
    using std::hypot;
    using std::fma;
    static_assert(std::is_same<decltype(hypot(user::X(), user::Y())), user::X>::value, "ADL");
    static_assert(std::is_same<decltype(fma(user::X(), user::Y(), user::X())), user::X>::value, "ADL");
    static_assert(std::is_same<std::int_fast32_t, ::int_fast32_t>::value, "libc ABI");
    static_assert(std::is_same<decltype(std::round(1)), double>::value, "promotion");
    static_assert(std::is_same<decltype(std::hypot(1.f, 2)), double>::value, "promotion");
    static_assert(std::is_same<decltype(std::fma(1.f, 2.f, 3.L)), long double>::value, "promotion");
    static_assert(std::is_same<decltype(std::isnan(0)), bool>::value, "predicate");
    static_assert(std::is_same<decltype(std::isgreater(0, 0.f)), bool>::value, "predicate");
    static_assert(INT64_C(9223372036854775807) == INT64_MAX, "integer macros");
    std::intmax_t big = INTMAX_MAX;
    char text[128], *end;
    std::sprintf(text, "%" PRIdMAX, big);
    assert(std::strtoimax(text, &end, 10) == big && *end == 0);
    std::uintmax_t unsigned_big = UINTMAX_MAX;
    std::sprintf(text, "%" PRIuMAX, unsigned_big);
    assert(std::strtoumax(text, &end, 10) == unsigned_big && *end == 0);
    assert(std::imaxabs(-42) == 42);
    std::imaxdiv_t qr = std::imaxdiv(-43, 5);
    assert(qr.quot == -8 && qr.rem == -3);
    assert(std::round(-2.5) == -3 && std::round(2.5f) == 3.f);
    assert(std::trunc(-2.9L) == -2.L && std::hypot(3, 4.f) == 5.);
    assert(std::fma(2, 3.f, 4.) == 10.);
    assert(std::signbit(-0.f) && std::signbit(-0.L));
    assert(!std::signbit(0.) && !std::signbit(1));
    assert(std::fpclassify(0) == FP_ZERO);
    assert(std::fpclassify(std::nextafter(0., 1.)) == FP_SUBNORMAL);
    assert(std::isnormal(std::numeric_limits<double>::min()));
    assert(std::isnormal(std::numeric_limits<long double>::max()));
    assert(std::isfinite(std::numeric_limits<long double>::max()));
    std::fenv_t environment;
    assert(std::feholdexcept(&environment) == 0);
    const double nan = std::nan("");
    assert(!std::isgreater(once(nan), once(1.)) && calls == 2);
    assert(std::isunordered(nan, 1) && !std::isless(nan, 1));
    assert(!std::islessgreater(nan, nan));
    assert(std::fetestexcept(FE_INVALID) == 0);
    assert(std::fesetround(FE_DOWNWARD) == 0);
    volatile double half = 2.75;
    assert(std::nearbyint(half) == 2.);
    assert(std::feraiseexcept(FE_DIVBYZERO) == 0);
    std::fexcept_t flags;
    assert(std::fegetexceptflag(&flags, FE_DIVBYZERO) == 0);
    assert(std::feclearexcept(FE_ALL_EXCEPT) == 0);
    assert(std::fesetexceptflag(&flags, FE_DIVBYZERO) == 0);
    assert(std::fetestexcept(FE_DIVBYZERO));
    assert(std::fesetenv(&environment) == 0);
    std::mbstate_t state = {};
    char16_t c16;
    char32_t c32;
    assert(std::mbrtoc16(&c16, "A", 1, &state) == 1 && c16 == u'A');
    assert(std::c16rtomb(text, c16, &state) == 1 && text[0] == 'A');
    assert(std::mbrtoc32(&c32, "A", 1, &state) == 1 && c32 == U'A');
    assert(std::c32rtomb(text, c32, &state) == 1 && text[0] == 'A');
    if (std::setlocale(LC_CTYPE, "C.UTF-8")) {
        const char smile[] = "\xf0\x9f\x99\x82";
        assert(std::mbrtoc32(&c32, smile, 4, &state) == 4 && c32 == 0x1f642);
        assert(std::c32rtomb(text, c32, &state) == 4 && std::memcmp(text, smile, 4) == 0);
        assert(std::mbrtoc16(&c16, smile, 4, &state) == 4 && c16 == 0xd83d);
        assert(std::mbrtoc16(&c16, "", 0, &state) == std::size_t(-3) && c16 == 0xde42);
        assert(std::c16rtomb(text, 0xd83d, &state) == 0);
        assert(std::c16rtomb(text, 0xde42, &state) == 4 && std::memcmp(text, smile, 4) == 0);
    }
}
