// Original regression: canonical U4 constants must retain their value in U8 arithmetic.
#include <cassert>
template<unsigned N> unsigned long long mask() {
    const bool fits = N < 64;
    return (1ULL << (fits ? N : 0)) - 1;
}
__attribute__((noinline)) unsigned long long adjusted(unsigned long long x) {
    return x - 1;
}
int main() {
    assert(mask<31>() == 0x7fffffffULL);
    assert(mask<32>() == 0xffffffffULL);
    assert(mask<33>() == 0x1ffffffffULL);
    assert(mask<63>() == 0x7fffffffffffffffULL);
    assert(mask<64>() == 0);
    assert(adjusted(0x100000000ULL) == 0xffffffffULL);
    assert(adjusted(0x80000001ULL) == 0x80000000ULL);
    assert(adjusted(0) == ~0ULL);
    const bool yes = true;
    const unsigned long long a = (1ULL << (yes ? 32 : 0)) - 1;
    assert(a == 0xffffffffULL);
}
