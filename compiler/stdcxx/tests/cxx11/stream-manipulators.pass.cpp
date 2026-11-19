// Original C++11 stream forwarding, locale manipulator and failure checks.
// UNSUPPORTED: c++98, c++03
#include <sstream>
#include <ios>
#include <iomanip>
#include <locale>
#include <string>
#include <type_traits>
#include <cassert>
#include <cstdio>
template<class C> struct Currency : std::moneypunct<C> {
    typedef std::basic_string<C> S;
    C do_decimal_point() const { return C('.'); }
    C do_thousands_sep() const { return C(','); }
    std::string do_grouping() const { return "\3"; }
    S do_curr_symbol() const { return S(1,C('$')); }
    S do_positive_sign() const { return S(); }
    S do_negative_sign() const { return S(1,C('-')); }
    int do_frac_digits() const { return 2; }
    std::money_base::pattern do_pos_format() const {
        std::money_base::pattern p = {{std::money_base::sign,std::money_base::symbol,
                                      std::money_base::value,std::money_base::none}};
        return p;
    }
    std::money_base::pattern do_neg_format() const { return do_pos_format(); }
};
struct ThrowMoney : std::money_put<char> {
    iter_type do_put(iter_type, bool, std::ios_base&, char_type, long double) const { throw 77; }
};
struct StopBuffer : std::streambuf {
    int_type overflow(int_type) { return traits_type::eof(); }
};
template<class T, class = void> struct can_insert : std::false_type {};
template<class T> struct can_insert<T, decltype(void(std::declval<std::ostringstream>() << std::declval<T>()))> : std::true_type {};
struct Unsupported {};
int main() {
    static_assert(!can_insert<Unsupported>::value, "constrain forwarding to valid insertions");
    static_assert(std::is_same<decltype(std::ostringstream() << 42), std::ostringstream&&>::value, "derived stream return");
    static_assert(std::is_same<decltype(std::istringstream() >> std::declval<int&>()), std::istringstream&&>::value, "derived stream return");
    assert((std::ostringstream() << "value=" << 42).str() == "value=42");
    int number = 0;
    (std::istringstream("2a") >> std::hex) >> number;
    assert(number == 42);
    std::ostringstream hexadecimal;
    hexadecimal.precision(1);
    hexadecimal << std::hexfloat << 1.5;
    assert(hexadecimal.str() == "0x1.8p+0");
    hexadecimal.str("");
    hexadecimal << std::uppercase << 1.5;
    assert(hexadecimal.str() == "0X1.8P+0");
    hexadecimal.str("");
    hexadecimal << std::defaultfloat << std::nouppercase << 1.5;
    assert(hexadecimal.str() == "2"); // Restored default format honors precision.
    std::tm t = {};
    std::istringstream input("2024-02-29 07:08:09");
    input >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");
    assert(!input.fail() && t.tm_year == 124 && t.tm_mon == 1 && t.tm_mday == 29);
    assert(t.tm_hour == 7 && t.tm_min == 8 && t.tm_sec == 9);
    std::ostringstream output;
    output << std::put_time(&t, "%Y-%m-%d %H:%M:%S");
    assert(output.str() == "2024-02-29 07:08:09");
    std::wistringstream wide_input(L"2024-02-29");
    wide_input >> std::get_time(&t, L"%Y-%m-%d");
    assert(!wide_input.fail());
    std::wostringstream wide_output;
    wide_output << std::put_time(&t, L"%Y-%m-%d");
    assert(wide_output.str() == L"2024-02-29");
    const std::locale money(std::locale::classic(), new Currency<char>);
    std::ostringstream amount;
    amount.imbue(money);
    amount << std::showbase << std::put_money(-123456.L);
    assert(amount.str() == "-$1,234.56");
    std::istringstream parse(amount.str()); parse.imbue(money);
    long double cents = 0;
    parse >> std::showbase >> std::get_money(cents);
    assert(!parse.fail() && cents == -123456.L);
    std::string digits;
    std::istringstream parse_digits("-$1,234.56"); parse_digits.imbue(money);
    parse_digits >> std::get_money(digits);
    assert(!parse_digits.fail() && digits == "-123456");
    std::ostringstream exceptional;
    exceptional.imbue(std::locale(std::locale::classic(), new ThrowMoney));
    exceptional << std::put_money(1.L);
    assert(exceptional.bad());
    exceptional.clear(); exceptional.exceptions(std::ios_base::badbit);
    try { exceptional << std::put_money(1.L); assert(false); }
    catch(int n) { assert(n == 77 && exceptional.bad()); }
    StopBuffer stopped;
    std::ostream full(&stopped);
    full << std::put_time(&t, "%Y"); assert(full.bad());
    std::istringstream invalid("not a year");
    invalid >> std::get_time(&t, "%Y"); assert(invalid.fail());
}
