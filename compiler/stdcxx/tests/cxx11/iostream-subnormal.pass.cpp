// Original regression: numeric extraction must preserve subnormals and zero.
// UNSUPPORTED: c++98, c++03
#include <sstream>
#include <limits>
#include <cmath>
#include <cassert>

template<class T,class C> void roundtrip() {
    if (std::numeric_limits<T>::has_denorm==std::denorm_absent) return;
    const T tiny=std::numeric_limits<T>::denorm_min();
    const T values[]={tiny,-tiny,tiny*T(3),-tiny*T(7),std::numeric_limits<T>::min()/T(2)};
    for(T expected:values) {
        std::basic_stringstream<C> stream;
        stream.precision(std::numeric_limits<T>::max_digits10);
        stream<<expected<<stream.widen(' ');
        T actual=42; stream>>actual;
        assert(!stream.fail() && actual==expected);
        stream.exceptions(std::ios::failbit|std::ios::badbit);
    }
}
template<class T> void limits() {
    std::istringstream tiny("-1e-99999"); T value=42; tiny>>value;
    assert(value==0 && std::signbit(value));
    std::istringstream huge("1e99999"); value=0; huge>>value; assert(huge.fail());
    std::istringstream negative("-1e99999"); value=0; negative>>value; assert(negative.fail());
    std::istringstream invalid("bad"); value=42; invalid>>value; assert(invalid.fail() && value==0);
}
int main() {
    roundtrip<float,char>(); roundtrip<double,char>(); roundtrip<long double,char>();
    roundtrip<float,wchar_t>(); roundtrip<double,wchar_t>(); roundtrip<long double,wchar_t>();
    limits<float>(); limits<double>(); limits<long double>();
}
