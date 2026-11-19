// Original C++11 numeric-string boundary, exception and formatting checks.
// UNSUPPORTED: c++98, c++03
#include <string>
#include <stdexcept>
#include <limits>
#include <cerrno>
#include <cmath>
#include <cassert>
#include <cstdio>
#include <cwchar>

int main() {
    const long long lo = (std::numeric_limits<long long>::min)();
    const long long hi = (std::numeric_limits<long long>::max)();
    const unsigned long long umax = (std::numeric_limits<unsigned long long>::max)();
    assert(std::atoll("-1234567890123") == -1234567890123LL);
    assert(std::llabs(-1234567890123LL) == 1234567890123LL);
    const std::lldiv_t division = std::div(hi, 3LL);
    assert(division.quot == hi/3 && division.rem == hi%3);
    assert(std::strtof("1.5", 0) == 1.5f && std::strtold("2.5", 0) == 2.5L);
    assert(std::stoll(std::to_string(lo)) == lo);
    assert(std::stoll(std::to_string(hi)) == hi);
    assert(std::stoull(std::to_string(umax)) == umax);
    assert(std::stoll(std::to_wstring(lo)) == lo);
    assert(std::stoll(std::to_wstring(hi)) == hi);
    assert(std::stoull(std::to_wstring(umax)) == umax);
    assert(std::stoul("-1") == (std::numeric_limits<unsigned long>::max)());
    assert(std::stoull(L"-1") == umax);
    std::size_t pos = 99;
    errno = EDOM;
    assert(std::stoi("  -0x2az", &pos, 0) == -42 && pos == 7 && errno == EDOM);
    assert(std::stoi(L"101stop", &pos, 2) == 5 && pos == 3 && errno == EDOM);
    assert(std::stod("1.25tail", &pos) == 1.25 && pos == 4 && errno == EDOM);
    assert(std::stof(L"0x1.8p+2tail", &pos) == 6.f && pos == 8 && errno == EDOM);
    const char* invalid[] = {"", "  ", "+", "-x"};
    for (unsigned i = 0; i < sizeof invalid/sizeof *invalid; ++i) {
        pos = 99;
        try { std::stoi(invalid[i], &pos); assert(false); }
        catch (const std::invalid_argument&) { assert(pos == 99 && errno == EDOM); }
        try { std::stod(invalid[i], &pos); assert(false); }
        catch (const std::invalid_argument&) { assert(pos == 99 && errno == EDOM); }
    }
    const char* outside[] = {"9223372036854775808", "-9223372036854775809"};
    for (unsigned i = 0; i < sizeof outside/sizeof *outside; ++i) {
        pos = 99;
        try { std::stoll(outside[i], &pos); assert(false); }
        catch (const std::out_of_range&) { assert(pos == 99 && errno == EDOM); }
    }
    pos = 99;
    try { std::stoull(L"18446744073709551616", &pos); assert(false); }
    catch (const std::out_of_range&) { assert(pos == 99 && errno == EDOM); }
    try { std::stoi(std::to_string(hi), &pos); assert(false); }
    catch (const std::out_of_range&) { assert(pos == 99 && errno == EDOM); }
    try { std::stof("1e-9999", &pos); assert(false); }
    catch (const std::out_of_range&) { assert(pos == 99 && errno == EDOM); }
    try { std::stold(L"1e99999", &pos); assert(false); }
    catch (const std::out_of_range&) { assert(pos == 99 && errno == EDOM); }
    const std::string embedded("12\0oops", 7);
    assert(std::stoi(embedded, &pos) == 12 && pos == 2);
    assert(std::signbit(std::stod("-0")));
    assert(std::isinf(std::stod("-inf")) && std::isnan(std::stold(L"nan")));
    assert(std::to_string(-0.) == "-0.000000");
    assert(std::to_wstring(1.25f) == L"1.250000");
    // These exceed the small formatting buffer; compare directly with libc.
    const double large = (std::numeric_limits<double>::max)();
    char bytes[8192];
    wchar_t wide[8192];
    int n = std::snprintf(bytes, sizeof bytes, "%f", large);
    assert(n > 128 && std::to_string(large) == bytes);
    n = std::swprintf(wide, 8192, L"%f", large);
    assert(n > 128 && std::to_wstring(large) == wide);
    const long double larger = (std::numeric_limits<long double>::max)();
    n = std::snprintf(bytes, sizeof bytes, "%Lf", larger);
    assert(n > 128 && std::to_string(larger) == bytes);
    n = std::swprintf(wide, 8192, L"%Lf", larger);
    assert(n > 128 && std::to_wstring(larger) == wide);
}
