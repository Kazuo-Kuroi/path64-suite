// Original generic-iterator, range-state and COW string endpoint regressions.
// UNSUPPORTED: c++98, c++03
#include <locale>
#include <sstream>
#include <string>
#include <limits>
#include <cassert>
struct Input {
    typedef std::input_iterator_tag iterator_category;
    typedef char value_type;
    typedef std::ptrdiff_t difference_type;
    typedef const char* pointer;
    typedef const char& reference;
    const char* p;
    explicit Input(const char* s) : p(s) {}
    char operator*() const { return *p; }
    Input& operator++() { ++p; return *this; }
    Input operator++(int) { Input old(*this); ++*this; return old; }
    bool operator==(Input x) const { return p==x.p; }
    bool operator!=(Input x) const { return p!=x.p; }
    template<class T> void operator,(const T&)=delete;
};
struct Facet : std::num_get<char,Input> { Facet() : std::num_get<char,Input>(1) {} };
struct Grouping : std::numpunct<char> {
    char do_thousands_sep() const { return '_'; }
    std::string do_grouping() const { return "\1\2\3"; }
};
int main() {
    Facet f;
    std::istringstream ios;
    ios.setf(std::ios::boolalpha);
    const char text[]="true!";
    std::ios::iostate state=std::ios::goodbit;
    bool b=false;
    Input end=f.get(Input(text),Input(text+5),ios,state,b);
    assert(b && state==std::ios::goodbit && end.p==text+4);
    ios.unsetf(std::ios::boolalpha);
    const char digits[]="12345!";
    long n=0; state=std::ios::goodbit;
    end=f.get(Input(digits),Input(digits+6),ios,state,n);
    assert(n==12345 && state==std::ios::goodbit && end.p==digits+5);
    std::ostringstream max;
    max << std::numeric_limits<unsigned>::max();
    std::string overflow=max.str(); ++overflow.back();
    std::istringstream in(overflow);
    unsigned u=0; in>>u;
    assert(in.fail() && u==std::numeric_limits<unsigned>::max());
    std::istringstream valid(max.str()); valid>>u;
    assert(!valid.fail() && u==std::numeric_limits<unsigned>::max());
    std::istringstream negative("-1"); unsigned short us=0;
    negative>>us; assert(!negative.fail() && us==std::numeric_limits<unsigned short>::max());
    std::istringstream invalid("-"); long old=42;
    invalid>>old; assert(invalid.fail() && old==0);
    std::istringstream numeric_bool("12"); bool truth=false;
    numeric_bool>>truth; assert(numeric_bool.fail() && truth);
    std::istringstream floats("0x1.8p+1 inf nan");
    double d=0; floats>>d; assert(d==3); floats>>d;
    assert(d==std::numeric_limits<double>::infinity());
    floats>>d; assert(d!=d);
    std::string huge(4096,'9');
    old=0; state=std::ios::goodbit;
    end=f.get(Input(huge.data()),Input(huge.data()+huge.size()),ios,state,old);
    assert(end.p==huge.data()+huge.size() && (state&std::ios::failbit));
    ios.imbue(std::locale(std::locale::classic(),new Grouping));
    const char grouped[]="+_1_";
    old=42; state=std::ios::goodbit;
    end=f.get(Input(grouped),Input(grouped+4),ios,state,old);
    assert(end.p==grouped+4 && (state&std::ios::failbit) && old==1);
    std::string a("abc"), copy(a);
    a.front()='x'; a.back()='z';
    assert(a=="xbz" && copy=="abc");
    const std::string& c=copy;
    assert(c.front()=='a' && c.back()=='c');
    std::wstring wide(L"xy"), other(wide);
    wide.front()=L'a'; wide.back()=L'b';
    assert(wide==L"ab" && other==L"xy");
}
