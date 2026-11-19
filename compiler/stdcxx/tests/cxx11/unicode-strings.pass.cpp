// Original Unicode code-unit traits, string aliases and hash regressions.
// UNSUPPORTED: c++98, c++03
#include <iosfwd>
#include <string>
#include <functional>
#include <cstdint>
#include <type_traits>
#include <cassert>
#include <sys/mman.h>
#include <unistd.h>

static_assert(std::char_traits<char>::to_int_type(char(-1)) == 255, "unsigned narrow conversion");
static_assert(std::char_traits<char>::not_eof(std::char_traits<char>::eof()) == 0, "narrow EOF");
static_assert(std::char_traits<wchar_t>::eq(L'a', L'a'), "wide equality");
static_assert(std::is_same<std::char_traits<char16_t>::int_type, std::uint_least16_t>::value, "16-bit int type");
static_assert(std::is_same<std::char_traits<char32_t>::int_type, std::uint_least32_t>::value, "32-bit int type");
static_assert(std::is_same<std::char_traits<char16_t>::pos_type, std::u16streampos>::value, "16-bit position");
static_assert(std::is_same<std::char_traits<char32_t>::pos_type, std::u32streampos>::value, "32-bit position");
static_assert(std::char_traits<char16_t>::eq(u'\u1234', u'\u1234'), "constant equality");
static_assert(std::char_traits<char32_t>::lt(U'\U00010000', U'\U0001f642'), "constant order");
static_assert(std::char_traits<char16_t>::to_int_type(u'\u1234') == 0x1234, "constant conversion");
static_assert(std::char_traits<char32_t>::not_eof(std::char_traits<char32_t>::eof()) != std::char_traits<char32_t>::eof(), "constant EOF");

template<class Char>
void traits() {
    typedef std::char_traits<Char> T;
    Char a[] = {Char(0x100), Char(1), Char(0), Char(0xffff), Char(0)};
    Char b[] = {Char(1), Char(0x100), Char(0)};
    assert(T::compare(a, b, 2) > 0); // Numeric order, not byte order.
    assert(T::compare(a, b, 0) == 0);
    assert(T::length(a) == 2);
    assert(T::find(a, 2, Char(0)) == 0);
    assert(T::find(a, 3, Char(0)) == a+2);
    assert(T::find(0, 0, Char(1)) == 0);
    assert(T::move(0, 0, 0) == 0 && T::copy(0, 0, 0) == 0);
    assert(T::assign(static_cast<Char*>(0), 0, Char(1)) == 0);
    Char x[] = {Char(1), Char(2), Char(3), Char(4), Char(5)};
    assert(T::move(x+1, x, 4) == x+1);
    for (int i = 1; i < 5; ++i) assert(x[i] == Char(i));
    T::move(x, x+1, 4);
    for (int i = 0; i < 4; ++i) assert(x[i] == Char(i+1));
    T::assign(x, 5, Char(0x8123));
    for (int i = 0; i < 5; ++i) assert(x[i] == Char(0x8123));
    // A zero-length/failed search must never inspect the following page.
    long page = ::sysconf(_SC_PAGESIZE);
    assert(page > 0);
    void* area = ::mmap(0, std::size_t(page)*2, PROT_READ|PROT_WRITE,
                        MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    assert(area != MAP_FAILED);
    assert(::mprotect(static_cast<char*>(area)+page, page, PROT_NONE) == 0);
    Char* end = reinterpret_cast<Char*>(static_cast<char*>(area)+page);
    end[-1] = Char(5);
    assert(T::find(end-1, 1, Char(7)) == 0);
    assert(T::find(end, 0, Char(7)) == 0);
    assert(::munmap(area, std::size_t(page)*2) == 0);
}
template<class String>
void strings(const typename String::value_type* text) {
    typedef typename String::value_type C;
    String a(text), b(a);
    assert(a == b && a.size() == 4 && a.data()[a.size()] == C());
    a[0] = C('x');
    assert(b[0] == text[0]); // Copies must not share mutable characters.
    a.append(b);
    assert(a.size() == 8 && a.substr(4) == b);
    a.insert(1, b);
    assert(a.size() == 12 && a.substr(1, 4) == b);
    a.erase(1, 4);
    assert(a.size() == 8);
    a.replace(0, 4, b);
    assert(a.substr(0, 4) == b && a.find(b) == 0 && a.rfind(b) == 4);
    String joined = b + b;
    assert(joined == a);
    joined.reserve(256);
    joined.resize(24, C(0x1234));
    assert(joined[23] == C(0x1234) && joined.data()[24] == C());
    std::hash<String> hash;
    assert(hash(b) == hash(String(text)));
    const C units[] = {C('a'), C(0), C(0xffff), C('b')};
    String embedded(units, 4);
    assert(embedded.size() == 4 && embedded[1] == C(0) && embedded[2] == C(0xffff));
    assert(embedded.find(C(0)) == 1 && embedded.substr(2).size() == 2);
    assert(hash(embedded) == hash(String(units, 4)));
    String empty;
    assert(empty.empty() && empty.c_str()[0] == C());
    empty.swap(b);
    assert(empty.size() == 4 && b.empty());
}
int main() {
    traits<char16_t>(); traits<char32_t>();
    strings<std::u16string>(u"A\u1234\U0001f642"); // UTF-16 surrogate pair.
    strings<std::u32string>(U"A\u1234\U0001f642Z");
}
