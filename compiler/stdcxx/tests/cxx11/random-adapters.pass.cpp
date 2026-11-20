// Original engine-adapter boundaries, exact table indexing and I/O rollback.
// UNSUPPORTED: c++98, c++03
#include <random>
#include <sstream>
#include <cassert>
#include <climits>
struct Counter {
    typedef unsigned long long result_type;
    result_type next;
    Counter(result_type seed=0) : next(seed) {}
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return ULLONG_MAX; }
    result_type operator()() { return next++; }
    void seed(result_type value=0) { next=value; }
    void discard(unsigned long long n) { next+=n; }
    friend bool operator==(Counter x,Counter y) { return x.next==y.next; }
    friend bool operator!=(Counter x,Counter y) { return !(x==y); }
};
struct Offset {
    typedef unsigned result_type;
    unsigned next;
    Offset(unsigned n=10) : next(n) {}
    static constexpr unsigned min() { return 10; }
    static constexpr unsigned max() { return 15; }
    unsigned operator()() { unsigned out=next; next=out==15?10:out+1; return out; }
    friend bool operator==(Offset x,Offset y) { return x.next==y.next; }
};
template<class E> void streams() {
    E e; e.discard(10); E saved=e;
    std::stringstream out; out.setf(std::ios::hex,std::ios::basefield); out.fill('*'); out.width(20);
    auto flags=out.flags(); out<<e;
    assert(out.flags()==flags && out.fill()=='*');
    E copy; out>>copy; assert(copy==saved && out.flags()==flags);
    for (int i=0;i<20;++i) assert(copy()==e());
    std::istringstream truncated("1");
    truncated.setf(std::ios::hex,std::ios::basefield); auto input_flags=truncated.flags();
    E original=e; truncated>>e; assert(truncated.fail() && e==original && truncated.flags()==input_flags);
    std::istringstream throwing("1"); throwing.setf(std::ios::hex,std::ios::basefield);
    throwing.exceptions(std::ios::failbit); input_flags=throwing.flags();
    try { throwing>>e; assert(false); } catch(const std::ios_base::failure&) {}
    assert(e==original && throwing.flags()==input_flags);
}
int main() {
    std::discard_block_engine<Counter,5,2> block;
    assert(block()==0 && block()==1 && block()==5 && block()==6);
    auto saved=block; block.discard(7); for(int i=0;i<7;++i) saved(); assert(block==saved);
    block.seed(10); assert(block()==10 && block()==11 && block()==15);
    std::independent_bits_engine<Counter,64,unsigned long long> wide(Counter(ULLONG_MAX));
    assert(wide()==ULLONG_MAX && wide()==0);
    // Six input values require rejection of 14 and 15 for a two-bit output.
    std::independent_bits_engine<Offset,2,unsigned> narrow(Offset(14));
    assert(narrow()==0 && narrow.base().next==11);
    std::shuffle_order_engine<Counter,3> table(Counter(ULLONG_MAX-3));
    assert(table()==ULLONG_MAX-1); // floor(3*ULLONG_MAX/2^64) == 2
    assert(table()==0); // selected slot is replaced by the wrapping next value
    for(unsigned r=2;r<100;++r)
        for(unsigned k=1;k<32;++k)
            for(unsigned v=0;v<r;++v) assert(std::__rw_random::__scaled_index(k,v,r)==k*v/r);
    assert(std::__rw_random::__scaled_index(3,ULLONG_MAX-1,ULLONG_MAX)==2);
    assert(std::__rw_random::__scaled_index(3,ULLONG_MAX,0)==2);
    assert(std::__rw_random::__scaled_index(3,1ULL<<63,0)==1);
    streams<std::ranlux24>(); streams<std::ranlux48>(); streams<std::knuth_b>();
    std::seed_seq seq{7,13}; std::knuth_b a(seq),b(seq); assert(a==b); a.discard(100); b.discard(100); assert(a==b);
}
