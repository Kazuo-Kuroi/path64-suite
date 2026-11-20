// Original random engine, range-boundary, rejection and shuffle regression.
// UNSUPPORTED: c++98, c++03
#include <random>
#include <algorithm>
#include <cassert>
#include <sstream>
#include <limits>
#include <climits>
struct Binary {
    typedef unsigned result_type;
    unsigned calls;
    Binary() : calls(0) {}
    static constexpr unsigned min() { return 5; }
    static constexpr unsigned max() { return 6; }
    unsigned operator()() { return 5+(calls++%2); }
};
struct Full {
    typedef unsigned long long result_type;
    unsigned calls;
    Full() : calls(0) {}
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return ULLONG_MAX; }
    result_type operator()() { return calls++%2?ULLONG_MAX:0; }
};
struct Reject {
    typedef unsigned result_type;
    unsigned calls;
    Reject() : calls(0) {}
    static constexpr unsigned min() { return 10; }
    static constexpr unsigned max() { return 15; }
    unsigned operator()() { const unsigned values[]={15,14,12}; return values[calls++%3]; }
};
template<class E> void roundtrip() {
    E e; e.discard(123); E copy=e;
    assert(copy==e && copy()==e());
    std::stringstream stream;
    stream<<e; E restored; stream>>restored;
    assert(restored==e);
    for (int i=0;i<100;++i) assert(restored()==e());
}
int main() {
    std::mt19937 mt; mt.discard(9999); assert(mt()==4123659995UL);
    std::mt19937_64 wide; wide.discard(9999); assert(wide()==9981545732273789042ULL);
    roundtrip<std::minstd_rand>(); roundtrip<std::mt19937>();
    roundtrip<std::mt19937_64>(); roundtrip<std::ranlux24_base>(); roundtrip<std::ranlux48_base>();
    std::seed_seq seq{1,2,3,4}; unsigned a[10],b[10]; seq.generate(a,a+10); seq.generate(b,b+10);
    assert(std::equal(a,a+10,b));
    std::mt19937 seeded(seq),other(seq); assert(seeded==other);
    Binary binary;
    std::uniform_int_distribution<unsigned long long> across(0,ULLONG_MAX);
    assert(across(binary)==0x5555555555555555ULL && binary.calls==64);
    Full full;
    std::uniform_int_distribution<long long> signed_range(LLONG_MIN,LLONG_MAX);
    assert(signed_range(full)==LLONG_MIN && signed_range(full)==LLONG_MAX);
    std::uniform_int_distribution<int> singleton(17,17); unsigned calls=full.calls;
    assert(singleton(full)==17 && full.calls==calls);
    Reject reject; std::uniform_int_distribution<unsigned> small(0,3);
    assert(small(reject)==2 && reject.calls==3);
    int counts[7]={}; std::mt19937 gen(42); std::uniform_int_distribution<int> bounded(-3,3);
    std::uniform_real_distribution<double> real(-10,10);
    for (int i=0;i<7000;++i) { int value=bounded(gen); assert(value>=-3 && value<=3); ++counts[value+3]; double x=real(gen); assert(x>=-10 && x<10); }
    for (int i=0;i<7;++i) assert(counts[i]>800 && counts[i]<1200);
    std::bernoulli_distribution never(0),always(1); assert(!never(gen) && always(gen));
    int values[64]; for (int i=0;i<64;++i) values[i]=i;
    std::shuffle(values,values+64,gen); assert(!std::is_sorted(values,values+64));
    std::sort(values,values+64); for (int i=0;i<64;++i) assert(values[i]==i);
    std::shuffle(values,values,gen); std::shuffle(values,values+1,std::mt19937());
    // An overflowing congruential multiply must still equal arithmetic mod m.
    typedef std::linear_congruential_engine<unsigned long long,18446744073709551555ULL,11,18446744073709551557ULL> Large;
    Large large(18446744073709551556ULL); assert(large()==13);
}
