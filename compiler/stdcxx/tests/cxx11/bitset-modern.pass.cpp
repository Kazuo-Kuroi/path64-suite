// Original bitset construction, conversion, shift-boundary and hash checks.
// UNSUPPORTED: c++98, c++03
#include <bitset>
#include <functional>
#include <limits>
#include <stdexcept>
#include <cassert>
#include <sys/mman.h>
#include <unistd.h>
constexpr std::bitset<0> empty_constant(123ULL);
constexpr std::bitset<65> wide_constant(0x8000000000000001ULL);
static_assert(empty_constant.size() == 0 && wide_constant.size() == 65, "constexpr constructors");
template<std::size_t N> void check() {
    const unsigned long long values[] = {0,1,0x8000000000000000ULL,0xaaaaaaaaaaaaaaaaULL,~0ULL};
    for (unsigned v=0; v<5; ++v) {
        std::bitset<N> b(values[v]);
        unsigned long long expected = 0;
        for (std::size_t i=0; i<N; ++i) {
            const bool bit = i<64 && ((values[v] >> (i%64)) & 1);
            assert(b[i] == bit);
            if (i<64 && bit) expected |= 1ULL << (i%64);
        }
        assert(b.to_ullong() == expected);
        const std::size_t shifts[] = {0,1,N,N+1,63,64,65,std::size_t(-1)};
        for (unsigned n=0; n<8; ++n) {
            const std::size_t shift = shifts[n];
            std::bitset<N> left(b), right(b);
            left <<= shift; right >>= shift;
            for (std::size_t i=0; i<N; ++i) {
                assert(left[i] == (shift<=i && b[i-shift]));
                assert(right[i] == (shift<N-i && b[i+shift]));
            }
        }
        std::bitset<N> same(b);
        same.flip().flip();
        assert(same == b && std::hash<std::bitset<N> >()(same) == std::hash<std::bitset<N> >()(b));
    }
    std::bitset<N> all;
    all.set(); assert(all.all() && all.count() == N);
}
int main() {
    check<0>(); check<1>(); check<31>(); check<32>(); check<33>();
    check<63>(); check<64>(); check<65>(); check<127>(); check<128>(); check<129>();
    std::bitset<0> zero;
    zero.set().flip();
    assert(zero.to_ulong() == 0 && zero.to_ullong() == 0 && zero.all());
    std::bitset<129> overflow;
    overflow.set(64);
    try { overflow.to_ullong(); assert(false); } catch(const std::overflow_error&) {}
    overflow.reset().set(128);
    try { overflow.to_ullong(); assert(false); } catch(const std::overflow_error&) {}
    assert(std::bitset<8>("1101", 2).to_ullong() == 3);
    assert(std::bitset<8>(u"abba", 4, u'a', u'b').to_ullong() == 6);
    try { std::bitset<8> bad("1012", 4); assert(false); } catch(const std::invalid_argument&) {}
    long page = sysconf(_SC_PAGESIZE);
    assert(page > 0);
    void* area = mmap(0, std::size_t(page)*2, PROT_READ|PROT_WRITE,
                      MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    assert(area != MAP_FAILED);
    char* end = static_cast<char*>(area)+page;
    end[-3]='1'; end[-2]='0'; end[-1]='1';
    assert(mprotect(end, page, PROT_NONE) == 0);
    assert(std::bitset<8>(end-3, 3).to_ullong() == 5);
    assert(munmap(area, std::size_t(page)*2) == 0);
}
