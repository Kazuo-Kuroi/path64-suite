// Original Path64 implementation of the standard random algorithms.
#ifndef _RWSTD_C11_RANDOM_INCLUDED
#define _RWSTD_C11_RANDOM_INCLUDED
#include <cstdint>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <vector>
#include <initializer_list>
#include <istream>
#include <ostream>
#include <cmath>
#include <utility>
#include <system_error>
#include <cstdlib>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <rw/_c11_random_bits.h>
namespace std {
namespace __rw_random {
template<class _U> constexpr _U __mask(unsigned __bits) {
    return __bits==numeric_limits<_U>::digits ? _U(~_U(0)) : (_U(1)<<__bits)-1;
}
// Addition and multiplication modulo m without an overflowing intermediate.
template<class _U> _U __add(_U __x,_U __y,_U __m) {
    return __x>=__m-__y ? __x-(__m-__y) : __x+__y;
}
template<class _U> _U __mul(_U __x,_U __y,_U __m) {
    if (!__m) return __x*__y;
    if (!__x || __y<=_U(~_U(0))/__x) return (__x*__y)%__m;
    _U __value=0;
    while (__y) {
        if (__y&1) __value=__add(__value,__x,__m);
        __y>>=1;
        if (__y) __x=__add(__x,__x,__m);
    }
    return __value;
}
template<class _S,class _R> using __seed_enable=typename enable_if<!is_convertible<_S,_R>::value && !is_same<typename decay<_S>::type,_R>::value,int>::type;
}
class seed_seq {
public:
    typedef uint_least32_t result_type;
    seed_seq() {}
    template<class _I> seed_seq(_I __first,_I __last) {
        for (;__first!=__last;++__first) __seeds.push_back(uint32_t(*__first));
    }
    template<class _T> seed_seq(initializer_list<_T> __values) : seed_seq(__values.begin(),__values.end()) {}
    seed_seq(const seed_seq&)=delete;
    seed_seq& operator=(const seed_seq&)=delete;
    size_t size() const noexcept { return __seeds.size(); }
    template<class _O> void param(_O __out) const {
        for (size_t __i=0;__i<size();++__i) *__out++=__seeds[__i];
    }
    template<class _I> void generate(_I __first,_I __last) {
        const size_t __n=__last-__first, __s=size();
        if (!__n) return;
        for (size_t __i=0;__i<__n;++__i) __first[__i]=uint32_t(0x8b8b8b8b);
        const size_t __t=__n>=623?11:__n>=68?7:__n>=39?5:__n>=7?3:(__n-1)/2;
        const size_t __p=(__n-__t)/2, __q=__p+__t, __m=__n>__s?__n:__s+1;
        for (size_t __k=0;__k<__m;++__k) {
            size_t __i=__k%__n, __j=(__k+__p)%__n, __z=(__k+__q)%__n;
            uint32_t __x=uint32_t(__first[__i])^uint32_t(__first[__j])^uint32_t(__first[(__k+__n-1)%__n]);
            uint32_t __r=uint32_t(1664525)*(__x^(__x>>27));
            __first[__j]=uint32_t(uint32_t(__first[__j])+__r);
            __r+=__k==0?uint32_t(__s):uint32_t(__i)+(__k<=__s?__seeds[__k-1]:0);
            __first[__z]=uint32_t(uint32_t(__first[__z])+__r);
            __first[__i]=__r;
        }
        for (size_t __k=__m;__k<__m+__n;++__k) {
            size_t __i=__k%__n, __j=(__k+__p)%__n, __z=(__k+__q)%__n;
            uint32_t __x=uint32_t(__first[__i])+uint32_t(__first[__j])+uint32_t(__first[(__k+__n-1)%__n]);
            uint32_t __r=uint32_t(1566083941)*(__x^(__x>>27));
            __first[__j]=uint32_t(__first[__j])^__r;
            __r-=uint32_t(__i);
            __first[__z]=uint32_t(__first[__z])^__r;
            __first[__i]=__r;
        }
    }
private:
    vector<uint32_t> __seeds;
};
template<class _U,_U __a,_U __c,_U __m> class linear_congruential_engine {
    static_assert(is_unsigned<_U>::value,"random engines require unsigned integers");
    static_assert(!__m || (__a<__m && __c<__m),"invalid congruential parameters");
    _U __state;
public:
    typedef _U result_type;
    static constexpr _U multiplier=__a, increment=__c, modulus=__m, default_seed=1;
    explicit linear_congruential_engine(_U __value=default_seed) { seed(__value); }
    template<class _S,typename enable_if<!is_convertible<_S,_U>::value && !is_same<_S,linear_congruential_engine>::value,int>::type =0>
    explicit linear_congruential_engine(_S& __seq) { seed(__seq); }
    void seed(_U __value=default_seed) {
        __state=__m?__value%__m:__value;
        if (!__c && !__state) __state=1;
    }
    template<class _S,typename enable_if<!is_convertible<_S,_U>::value && !is_same<_S,linear_congruential_engine>::value,int>::type =0> void seed(_S& __seq) {
        const size_t __k=(__m?__m-1>0xffffffffULL:numeric_limits<_U>::digits>32)?2:1;
        uint32_t __words[__k+3]; __seq.generate(__words,__words+__k+3);
        unsigned long long __value=__words[3];
        if (__k==2) __value|=static_cast<unsigned long long>(__words[4])<<32;
        seed(_U(__m?__value%__m:__value));
    }
    static constexpr _U min() { return __c?0:1; }
    static constexpr _U max() { return __m?__m-1:_U(~_U(0)); }
    _U operator()() {
        // Standard minstd parameters fit in a single native multiply. Keep
        // that common path free of the generic overflow test and fallback.
        _U __product;
        if (!__m) __product=_U(__a*__state);
        else if (!__a || __m-1<=_U(~_U(0))/(__a?__a:1)) __product=_U(__a*__state)%__m;
        else __product=__rw_random::__mul(_U(__a),__state,_U(__m));
        return __state=__m?__rw_random::__add(__product,_U(__c),_U(__m)):_U(__product+__c);
    }
    void discard(unsigned long long __n) { while (__n--) (*this)(); }
    friend bool operator==(const linear_congruential_engine& __x,const linear_congruential_engine& __y) { return __x.__state==__y.__state; }
    friend bool operator!=(const linear_congruential_engine& __x,const linear_congruential_engine& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __out,const linear_congruential_engine& __e) {
        typename basic_ostream<_C,_T>::fmtflags __flags=__out.flags();
        __out.flags(ios_base::dec|ios_base::left); __out<<__e.__state; __out.flags(__flags); return __out;
    }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __in,linear_congruential_engine& __e) {
        ios_base::fmtflags __flags=__in.flags(); __in.flags(ios_base::dec|ios_base::skipws);
        _U __value; if (__in>>__value) __e.__state=__value;
        __in.flags(__flags); return __in;
    }
};
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,
         size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f>
class mersenne_twister_engine {
    static_assert(is_unsigned<_U>::value && __w>=2 && __w<=numeric_limits<_U>::digits,"invalid twister word size");
    static_assert(__m>0 && __m<=__n && __r<=__w,"invalid twister state");
    _U __state[__n]; size_t __index;
    static constexpr _U __mask=__rw_random::__mask<_U>(__w);
    static _U __right(_U __x,size_t __shift) { return __shift>=numeric_limits<_U>::digits?0:__x>>__shift; }
    static _U __left(_U __x,size_t __shift) { return __shift>=numeric_limits<_U>::digits?0:__x<<__shift; }
public:
    typedef _U result_type;
    static constexpr size_t word_size=__w,state_size=__n,shift_size=__m,mask_bits=__r;
    static constexpr _U xor_mask=__a,tempering_d=__d,tempering_b=__b,tempering_c=__c,initialization_multiplier=__f,default_seed=5489;
    static constexpr size_t tempering_u=__u,tempering_s=__s,tempering_t=__t,tempering_l=__l;
    explicit mersenne_twister_engine(_U __value=default_seed) { seed(__value); }
    template<class _S,typename enable_if<!is_convertible<_S,_U>::value && !is_same<_S,mersenne_twister_engine>::value,int>::type =0> explicit mersenne_twister_engine(_S& __seq) { seed(__seq); }
    void seed(_U __value=default_seed) {
        __state[0]=__value&__mask;
        for (size_t __i=1;__i<__n;++__i)
            __state[__i]=(__f*(__state[__i-1]^__right(__state[__i-1],__w-2))+__i)&__mask;
        __index=0;
    }
    template<class _S,typename enable_if<!is_convertible<_S,_U>::value && !is_same<_S,mersenne_twister_engine>::value,int>::type =0> void seed(_S& __seq) {
        const size_t __k=(__w+31)/32;
        uint32_t __words[__n*__k]; __seq.generate(__words,__words+__n*__k);
        for (size_t __i=0;__i<__n;++__i) {
            __state[__i]=_U(__words[__i*__k]);
            if (__k==2) __state[__i]|=__left(_U(__words[__i*__k+1]),32);
            __state[__i]&=__mask;
        }
        bool __nonzero=(__state[0]&~__rw_random::__mask<_U>(__r))!=0;
        for (size_t __i=1;__i<__n;++__i) __nonzero=__nonzero || __state[__i]!=0;
        if (!__nonzero) __state[0]=_U(1)<<(__w-1);
        __index=0;
    }
    static constexpr _U min() { return 0; }
    static constexpr _U max() { return __mask; }
    _U operator()() {
        const _U __lower=__rw_random::__mask<_U>(__r);
        size_t __next=(__index+1)%__n;
        _U __joined=(__state[__index]&~__lower)|(__state[__next]&__lower);
        _U __value=__state[(__index+__m)%__n]^(__joined>>1)^((__joined&1)?__a:0);
        __state[__index]=__value; __index=__next;
        __value^=__right(__value,__u)&__d;
        __value^=__left(__value,__s)&__b;
        __value^=__left(__value,__t)&__c;
        __value^=__right(__value,__l);
        return __value&__mask;
    }
    void discard(unsigned long long __count) { while (__count--) (*this)(); }
    friend bool operator==(const mersenne_twister_engine& __x,const mersenne_twister_engine& __y) {
        for (size_t __i=0;__i<__n;++__i)
            if (__x.__state[(__x.__index+__i)%__n]!=__y.__state[(__y.__index+__i)%__n]) return false;
        return true;
    }
    friend bool operator!=(const mersenne_twister_engine& __x,const mersenne_twister_engine& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __out,const mersenne_twister_engine& __e) {
        ios_base::fmtflags __flags=__out.flags(); __out.flags(ios_base::dec|ios_base::left);
        for (size_t __i=0;__i<__n;++__i) {
            if (__i) __out.put(__out.widen(' '));
            __out<<__e.__state[(__e.__index+__i)%__n];
        }
        __out.flags(__flags); return __out;
    }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __in,mersenne_twister_engine& __e) {
        ios_base::fmtflags __flags=__in.flags(); __in.flags(ios_base::dec|ios_base::skipws);
        mersenne_twister_engine __copy=__e;
        for (size_t __i=0;__i<__n;++__i) if (!(__in>>__copy.__state[__i])) { __in.flags(__flags); return __in; }
        __copy.__index=0; __e=__copy; __in.flags(__flags); return __in;
    }
};
typedef linear_congruential_engine<uint_fast32_t,16807,0,2147483647> minstd_rand0;
typedef linear_congruential_engine<uint_fast32_t,48271,0,2147483647> minstd_rand;
typedef mersenne_twister_engine<uint_fast32_t,32,624,397,31,0x9908b0df,11,0xffffffff,7,0x9d2c5680,15,0xefc60000,18,1812433253> mt19937;
typedef mersenne_twister_engine<uint_fast64_t,64,312,156,31,0xb5026f5aa96619e9ULL,29,0x5555555555555555ULL,17,0x71d67fffeda60000ULL,37,0xfff7eee000000000ULL,43,6364136223846793005ULL> mt19937_64;
template<class _U,size_t __w,size_t __s,size_t __r> class subtract_with_carry_engine {
    static_assert(is_unsigned<_U>::value && __w>0 && __w<=numeric_limits<_U>::digits && __s>0 && __s<__r,"invalid subtract-with-carry parameters");
    _U __state[__r]; unsigned __carry; size_t __index;
    void __finish_seed() { __carry=__state[__r-1]==0; __index=0; }
public:
    typedef _U result_type;
    static constexpr size_t word_size=__w,short_lag=__s,long_lag=__r;
    static constexpr _U default_seed=19780503;
    explicit subtract_with_carry_engine(_U __value=default_seed) { seed(__value); }
    template<class _S,typename enable_if<!is_convertible<_S,_U>::value && !is_same<_S,subtract_with_carry_engine>::value,int>::type =0>
    explicit subtract_with_carry_engine(_S& __seq) { seed(__seq); }
    void seed(_U __value=default_seed) {
        linear_congruential_engine<uint_least32_t,40014,0,2147483563> __g(__value?__value:default_seed);
        for (size_t __i=0;__i<__r;++__i) {
            unsigned long long __x=__g();
            if (__w>32) __x|=static_cast<unsigned long long>(__g())<<32;
            __state[__i]=_U(__x)&max();
        }
        __finish_seed();
    }
    template<class _S,typename enable_if<!is_convertible<_S,_U>::value && !is_same<_S,subtract_with_carry_engine>::value,int>::type =0>
    void seed(_S& __seq) {
        const size_t __k=(__w+31)/32;
        uint32_t __words[__k*__r]; __seq.generate(__words,__words+__k*__r);
        for (size_t __i=0;__i<__r;++__i) {
            unsigned long long __x=__words[__i*__k];
            if (__k==2) __x|=static_cast<unsigned long long>(__words[__i*__k+1])<<32;
            __state[__i]=_U(__x)&max();
        }
        __finish_seed();
    }
    static constexpr _U min() { return 0; }
    static constexpr _U max() { return __rw_random::__mask<_U>(__w); }
    _U operator()() {
        _U __short=__state[(__index+__r-__s)%__r],__long=__state[__index];
        _U __value=_U(__short-__long-__carry)&max();
        __carry=__short<__long || (__short==__long && __carry);
        __state[__index]=__value; __index=(__index+1)%__r;
        return __value;
    }
    void discard(unsigned long long __n) { while (__n--) (*this)(); }
    friend bool operator==(const subtract_with_carry_engine& __x,const subtract_with_carry_engine& __y) {
        if (__x.__carry!=__y.__carry) return false;
        for (size_t __i=0;__i<__r;++__i) if (__x.__state[(__x.__index+__i)%__r]!=__y.__state[(__y.__index+__i)%__r]) return false;
        return true;
    }
    friend bool operator!=(const subtract_with_carry_engine& __x,const subtract_with_carry_engine& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __out,const subtract_with_carry_engine& __e) {
        ios_base::fmtflags __flags=__out.flags(); __out.flags(ios_base::dec|ios_base::left);
        for (size_t __i=0;__i<__r;++__i) __out<<__e.__state[(__e.__index+__i)%__r]<<__out.widen(' ');
        __out<<__e.__carry; __out.flags(__flags); return __out;
    }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __in,subtract_with_carry_engine& __e) {
        ios_base::fmtflags __flags=__in.flags(); __in.flags(ios_base::dec|ios_base::skipws);
        subtract_with_carry_engine __copy=__e;
        for (size_t __i=0;__i<__r;++__i) if (!(__in>>__copy.__state[__i])) { __in.flags(__flags); return __in; }
        if (__in>>__copy.__carry) { __copy.__index=0; __e=__copy; }
        __in.flags(__flags); return __in;
    }
};
typedef subtract_with_carry_engine<uint_fast32_t,24,10,24> ranlux24_base;
typedef subtract_with_carry_engine<uint_fast64_t,48,5,12> ranlux48_base;

namespace __rw_random {
// Formatted engine I/O has a decimal, whitespace-separated grammar, independent
// of caller flags. Restore formatting even if a stream exception interrupts it.
template<class _Stream> class __format_guard {
    _Stream& __stream;
    ios_base::fmtflags __flags;
    typename _Stream::char_type __fill;
    streamsize __precision;
public:
    __format_guard(_Stream& __s,ios_base::fmtflags __f)
        : __stream(__s),__flags(__s.flags()),__fill(__s.fill()),__precision(__s.precision()) {
        typename _Stream::char_type __space=__s.widen(' ');
        __s.flags(__f); __s.fill(__space);
    }
    ~__format_guard() { __stream.flags(__flags); __stream.fill(__fill); __stream.precision(__precision); }
    __format_guard(const __format_guard&)=delete;
    __format_guard& operator=(const __format_guard&)=delete;
};
// Compute floor(k*v/r) exactly, without overflowing k*v or requiring int128.
// A zero denominator denotes 2^64, the full-width generator range.
inline size_t __scaled_index(size_t __k,unsigned long long __v,unsigned long long __r) {
    size_t __q=0; unsigned long long __rem=0;
    size_t __bit=1;
    while (__bit<=__k/2) __bit<<=1;
    for (;__bit;__bit>>=1) {
        __q*=2;
        if (!__r) { bool __carry=(__rem>>63)!=0; __rem+=__rem; __q+=__carry; }
        else if (__rem>=__r-__rem) { __rem-=__r-__rem; ++__q; }
        else __rem+=__rem;
        if (__k&__bit) {
            if (!__r) { unsigned long long __sum=__rem+__v; __q+=__sum<__rem; __rem=__sum; }
            else if (__rem>=__r-__v) { __rem-=__r-__v; ++__q; }
            else __rem+=__v;
        }
    }
    return __q;
}
}
template<class _E,size_t __p,size_t __r> class discard_block_engine {
    _E __engine;
    size_t __used;
    void __initialize() { __used=0; }
public:
    typedef typename _E::result_type result_type;
    static_assert(__r>0 && __r<=__p,"invalid discard block");
    static constexpr size_t block_size=__p,used_block=__r;
    discard_block_engine() : __engine() { __initialize(); }
    explicit discard_block_engine(result_type __seed) : __engine(__seed) { __initialize(); }
    explicit discard_block_engine(const _E& __e) : __engine(__e) { __initialize(); }
    explicit discard_block_engine(_E&& __e) : __engine(std::move(__e)) { __initialize(); }
    template<class _S,typename enable_if<!is_convertible<_S,result_type>::value && !is_convertible<_S,_E>::value && !is_same<_S,discard_block_engine>::value,int>::type =0>
    explicit discard_block_engine(_S& __seq) : __engine(__seq) { __initialize(); }
    void seed() { __engine.seed(); __initialize(); }
    void seed(result_type __seed) { __engine.seed(__seed); __initialize(); }
    template<class _S,typename enable_if<!is_convertible<_S,result_type>::value,int>::type =0>
    void seed(_S& __seq) { __engine.seed(__seq); __initialize(); }
    const _E& base() const noexcept { return __engine; }
    static constexpr result_type min() { return _E::min(); }
    static constexpr result_type max() { return _E::max(); }
    result_type operator()() {
        if (__used==__r) { __engine.discard(__p-__r); __used=0; }
        ++__used; return __engine();
    }
    void discard(unsigned long long __n) { while (__n--) (*this)(); }
    friend bool operator==(const discard_block_engine& __x,const discard_block_engine& __y) { return __x.__engine==__y.__engine && __x.__used==__y.__used; }
    friend bool operator!=(const discard_block_engine& __x,const discard_block_engine& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __out,const discard_block_engine& __e) { __rw_random::__format_guard<basic_ostream<_C,_T> > __format(__out,ios_base::dec|ios_base::left); __out<<__e.__engine<<__out.widen(' ')<<__e.__used; return __out; }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __in,discard_block_engine& __e) {
        __rw_random::__format_guard<basic_istream<_C,_T> > __format(__in,ios_base::dec|ios_base::skipws);
        discard_block_engine __copy=__e; if (__in>>__copy.__engine>>__copy.__used) __e=__copy; return __in;
    }
};
template<class _E,size_t __p,size_t __r> constexpr size_t discard_block_engine<_E,__p,__r>::block_size;
template<class _E,size_t __p,size_t __r> constexpr size_t discard_block_engine<_E,__p,__r>::used_block;
template<class _E,size_t __w,class _U> class independent_bits_engine {
    _E __engine;

public:
    typedef _U result_type;
    static_assert(is_unsigned<_U>::value && __w>0 && __w<=numeric_limits<_U>::digits,"invalid independent bit width");
    independent_bits_engine() : __engine() {}
    explicit independent_bits_engine(result_type __seed) : __engine(__seed) {}
    explicit independent_bits_engine(const _E& __e) : __engine(__e) {}
    explicit independent_bits_engine(_E&& __e) : __engine(std::move(__e)) {}
    template<class _S,typename enable_if<!is_convertible<_S,result_type>::value && !is_convertible<_S,_E>::value && !is_same<_S,independent_bits_engine>::value,int>::type =0>
    explicit independent_bits_engine(_S& __seq) : __engine(__seq) {}
    void seed() { __engine.seed(); }
    void seed(result_type __seed) { __engine.seed(__seed); }
    template<class _S,typename enable_if<!is_convertible<_S,result_type>::value,int>::type =0>
    void seed(_S& __seq) { __engine.seed(__seq); }
    const _E& base() const noexcept { return __engine; }
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return __rw_random::__mask<_U>(__w); }
    result_type operator()() {
        typedef unsigned long long _Wide;
        const _Wide __range=_Wide((_E::max)())-_Wide((_E::min)())+1;
        if (!__range) return result_type(__engine())&max();
        unsigned __bits=0; for (_Wide __x=__range;__x>1;__x>>=1) ++__bits;
        size_t __draws=(__w+__bits-1)/__bits, __small=__w/__draws;
        _Wide __limit=(__range>>__small)<<__small;
        if (__range-__limit>__limit/__draws) { ++__draws; __small=__w/__draws; }
        const size_t __shorts=__draws-__w%__draws;
        result_type __value=0;
        for (size_t __i=0;__i<__draws;++__i) {
            unsigned __take=unsigned(__small+(__i>=__shorts));
            _Wide __bound=__take==64?0:(__range>>__take)<<__take, __sample;
            do { __sample=_Wide(__engine())-_Wide((_E::min)()); } while (__bound && __sample>=__bound);
            __value=__take>=numeric_limits<result_type>::digits?0:result_type(__value<<__take);
            __value|=result_type(__sample)&__rw_random::__mask<result_type>(__take);
        }
        return __value;
    }
    void discard(unsigned long long __n) { while (__n--) (*this)(); }
    friend bool operator==(const independent_bits_engine& __x,const independent_bits_engine& __y) { return __x.__engine==__y.__engine; }
    friend bool operator!=(const independent_bits_engine& __x,const independent_bits_engine& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __out,const independent_bits_engine& __e) { __rw_random::__format_guard<basic_ostream<_C,_T> > __format(__out,ios_base::dec|ios_base::left); return __out<<__e.__engine; }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __in,independent_bits_engine& __e) {
        __rw_random::__format_guard<basic_istream<_C,_T> > __format(__in,ios_base::dec|ios_base::skipws);
        independent_bits_engine __copy=__e; if (__in>>__copy.__engine) __e=__copy; return __in;
    }
};
template<class _E,size_t __k> class shuffle_order_engine {
    _E __engine;
    typename _E::result_type __table[__k],__last;
    void __initialize() { for (size_t __i=0;__i<__k;++__i) __table[__i]=__engine();
        __last=__engine(); }
public:
    typedef typename _E::result_type result_type;
    static_assert(__k>0,"empty shuffle table");
    static constexpr size_t table_size=__k;
    shuffle_order_engine() : __engine() { __initialize(); }
    explicit shuffle_order_engine(result_type __seed) : __engine(__seed) { __initialize(); }
    explicit shuffle_order_engine(const _E& __e) : __engine(__e) { __initialize(); }
    explicit shuffle_order_engine(_E&& __e) : __engine(std::move(__e)) { __initialize(); }
    template<class _S,typename enable_if<!is_convertible<_S,result_type>::value && !is_convertible<_S,_E>::value && !is_same<_S,shuffle_order_engine>::value,int>::type =0>
    explicit shuffle_order_engine(_S& __seq) : __engine(__seq) { __initialize(); }
    void seed() { __engine.seed(); __initialize(); }
    void seed(result_type __seed) { __engine.seed(__seed); __initialize(); }
    template<class _S,typename enable_if<!is_convertible<_S,result_type>::value,int>::type =0>
    void seed(_S& __seq) { __engine.seed(__seq); __initialize(); }
    const _E& base() const noexcept { return __engine; }
    static constexpr result_type min() { return _E::min(); }
    static constexpr result_type max() { return _E::max(); }
    result_type operator()() {
        const unsigned long long __range=static_cast<unsigned long long>((_E::max)())-static_cast<unsigned long long>((_E::min)())+1;
        size_t __index=__rw_random::__scaled_index(__k,static_cast<unsigned long long>(__last)-static_cast<unsigned long long>((_E::min)()),__range);
        __last=__table[__index]; __table[__index]=__engine(); return __last;
    }
    void discard(unsigned long long __n) { while (__n--) (*this)(); }
    friend bool operator==(const shuffle_order_engine& __x,const shuffle_order_engine& __y) { for (size_t __i=0;__i<__k;++__i) if (__x.__table[__i]!=__y.__table[__i]) return false; return __x.__engine==__y.__engine && __x.__last==__y.__last; }
    friend bool operator!=(const shuffle_order_engine& __x,const shuffle_order_engine& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __out,const shuffle_order_engine& __e) { __rw_random::__format_guard<basic_ostream<_C,_T> > __format(__out,ios_base::dec|ios_base::left); __out<<__e.__engine; for (size_t __i=0;__i<__k;++__i) __out<<__out.widen(' ')<<__e.__table[__i]; __out<<__out.widen(' ')<<__e.__last; return __out; }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __in,shuffle_order_engine& __e) {
        __rw_random::__format_guard<basic_istream<_C,_T> > __format(__in,ios_base::dec|ios_base::skipws);
        shuffle_order_engine __copy=__e; if (!(__in>>__copy.__engine)) return __in; for (size_t __i=0;__i<__k;++__i) if (!(__in>>__copy.__table[__i])) return __in; if (__in>>__copy.__last) __e=__copy; return __in;
    }
};
template<class _E,size_t __k> constexpr size_t shuffle_order_engine<_E,__k>::table_size;
typedef discard_block_engine<ranlux24_base,223,23> ranlux24;
typedef discard_block_engine<ranlux48_base,389,11> ranlux48;
typedef shuffle_order_engine<minstd_rand0,256> knuth_b;
typedef minstd_rand default_random_engine;
template<class _Real,size_t __bits,class _G> _Real generate_canonical(_G& __g) {
    const unsigned __wanted=__bits<unsigned(numeric_limits<_Real>::digits)?unsigned(__bits):unsigned(numeric_limits<_Real>::digits);
    _Real __range=_Real((_G::max)())-_Real((_G::min)())+_Real(1);
    _Real __total=0,__factor=1;
    unsigned __width=0;
    unsigned long long __span=static_cast<unsigned long long>((_G::max)())-static_cast<unsigned long long>((_G::min)())+1;
    if (!__span) __width=64;
    else while (__span>1) { ++__width; __span>>=1; }
    unsigned __draws=__wanted?(__wanted+__width-1)/__width:1;
    for (unsigned __i=0;__i<__draws;++__i) {
        __total+=(_Real(__g())-_Real((_G::min)()))*__factor;
        __factor*=__range;
    }
    _Real __value=_Real(__total/__factor);
    return __value<_Real(1)?__value:std::nextafter(_Real(1),_Real(0));
}
template<class _Int=int> class uniform_int_distribution {
public:
    typedef _Int result_type;
    class param_type {
        _Int __a,__b;
    public:
        typedef uniform_int_distribution distribution_type;
        explicit param_type(_Int __x=0,_Int __y=(numeric_limits<_Int>::max)()) : __a(__x),__b(__y) {}
        _Int a() const { return __a; } _Int b() const { return __b; }
        friend bool operator==(const param_type& __x,const param_type& __y) { return __x.__a==__y.__a && __x.__b==__y.__b; }
        friend bool operator!=(const param_type& __x,const param_type& __y) { return !(__x==__y); }
    };
private: param_type __param;
public:
    explicit uniform_int_distribution(_Int __a=0,_Int __b=(numeric_limits<_Int>::max)()) : __param(__a,__b) {}
    explicit uniform_int_distribution(const param_type& __p) : __param(__p) {}
    void reset() {}
    _Int a() const { return __param.a(); } _Int b() const { return __param.b(); }
    _Int min() const { return a(); } _Int max() const { return b(); }
    param_type param() const { return __param; } void param(const param_type& __p) { __param=__p; }
    template<class _G> _Int operator()(_G& __g) { return (*this)(__g,__param); }
    template<class _G> _Int operator()(_G& __g,const param_type& __p) {
        typedef typename make_unsigned<_Int>::type _U;
        _U __span=_U(_U(__p.b())-_U(__p.a())+_U(1));
        if (__span==1) return __p.a();
        unsigned __count=0; _U __n=__span-1;
        if (!__span) __count=numeric_limits<_U>::digits;
        else while (__n) { ++__count; __n>>=1; }
        _U __value;
        do { __value=_U(__rw_random::__bits(__g,__count)); } while (__span && __value>=__span);
        return _Int(_U(__p.a())+__value);
    }
    friend bool operator==(const uniform_int_distribution& __x,const uniform_int_distribution& __y) { return __x.__param==__y.__param; }
    friend bool operator!=(const uniform_int_distribution& __x,const uniform_int_distribution& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __o,const uniform_int_distribution& __d) { return __o<<__d.a()<<__o.widen(' ')<<__d.b(); }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __i,uniform_int_distribution& __d) { _Int __a,__b; if (__i>>__a>>__b) __d.param(param_type(__a,__b)); return __i; }
};
template<class _Real=double> class uniform_real_distribution {
public:
    typedef _Real result_type;
    class param_type {
        _Real __a,__b;
    public:
        typedef uniform_real_distribution distribution_type;
        explicit param_type(_Real __x=0,_Real __y=1) : __a(__x),__b(__y) {}
        _Real a() const { return __a; } _Real b() const { return __b; }
        friend bool operator==(const param_type& __x,const param_type& __y) { return __x.__a==__y.__a && __x.__b==__y.__b; }
        friend bool operator!=(const param_type& __x,const param_type& __y) { return !(__x==__y); }
    };
private: param_type __param;
public:
    explicit uniform_real_distribution(_Real __a=0,_Real __b=1) : __param(__a,__b) {}
    explicit uniform_real_distribution(const param_type& __p) : __param(__p) {}
    void reset() {}
    _Real a() const { return __param.a(); } _Real b() const { return __param.b(); }
    _Real min() const { return a(); } _Real max() const { return b(); }
    param_type param() const { return __param; } void param(const param_type& __p) { __param=__p; }
    template<class _G> _Real operator()(_G& __g) { return (*this)(__g,__param); }
    template<class _G> _Real operator()(_G& __g,const param_type& __p) {
        _Real __u=generate_canonical<_Real,numeric_limits<_Real>::digits>(__g);
        _Real __value=(_Real(1)-__u)*__p.a()+__u*__p.b();
        return __value<__p.b()?__value:std::nextafter(__p.b(),__p.a());
    }
    friend bool operator==(const uniform_real_distribution& __x,const uniform_real_distribution& __y) { return __x.__param==__y.__param; }
    friend bool operator!=(const uniform_real_distribution& __x,const uniform_real_distribution& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __o,const uniform_real_distribution& __d) {
        ios_base::fmtflags __flags=__o.flags(); streamsize __precision=__o.precision();
        __o.flags(ios_base::dec|ios_base::scientific|ios_base::left); __o.precision((2+numeric_limits<_Real>::digits*30103/100000));
        __o<<__d.a()<<__o.widen(' ')<<__d.b(); __o.flags(__flags); __o.precision(__precision); return __o;
    }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __i,uniform_real_distribution& __d) { _Real __a,__b; if (__i>>__a>>__b) __d.param(param_type(__a,__b)); return __i; }
};
class bernoulli_distribution {
public:
    typedef bool result_type;
    class param_type {
        double __p;
    public:
        typedef bernoulli_distribution distribution_type;
        explicit param_type(double __value=0.5) : __p(__value) {}
        double p() const { return __p; }
        friend bool operator==(const param_type& __x,const param_type& __y) { return __x.__p==__y.__p; }
        friend bool operator!=(const param_type& __x,const param_type& __y) { return !(__x==__y); }
    };
private: param_type __param;
public:
    explicit bernoulli_distribution(double __p=0.5) : __param(__p) {}
    explicit bernoulli_distribution(const param_type& __p) : __param(__p) {}
    void reset() {}
    double p() const { return __param.p(); } bool min() const { return false; } bool max() const { return true; }
    param_type param() const { return __param; } void param(const param_type& __p) { __param=__p; }
    template<class _G> bool operator()(_G& __g) { return (*this)(__g,__param); }
    template<class _G> bool operator()(_G& __g,const param_type& __p) { return generate_canonical<double,numeric_limits<double>::digits>(__g)<__p.p(); }
    friend bool operator==(const bernoulli_distribution& __x,const bernoulli_distribution& __y) { return __x.__param==__y.__param; }
    friend bool operator!=(const bernoulli_distribution& __x,const bernoulli_distribution& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __o,const bernoulli_distribution& __d) {
        ios_base::fmtflags __flags=__o.flags(); streamsize __precision=__o.precision();
        __o.flags(ios_base::dec|ios_base::scientific|ios_base::left); __o.precision((2+numeric_limits<double>::digits*30103/100000));
        __o<<__d.p(); __o.flags(__flags); __o.precision(__precision); return __o;
    }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __i,bernoulli_distribution& __d) { double __p; if (__i>>__p) __d.param(param_type(__p)); return __i; }
};
// A POSIX entropy source. Tokens are implementation-defined: this implementation
// accepts /dev/urandom only, and never substitutes a deterministic engine.
class random_device {
    int __fd;
    static void __fail(int __error) {
#if defined(__EXCEPTIONS) && !defined(_RWSTD_NO_EXCEPTIONS)
        throw system_error(__error,generic_category(),"random_device");
#else
        std::abort();
#endif
    }
public:
    typedef unsigned int result_type;
    explicit random_device(const string& __token="/dev/urandom") : __fd(-1) {
        if (__token!="/dev/urandom") __fail(EINVAL);
        int __flags=O_RDONLY;
#ifdef O_CLOEXEC
        __flags|=O_CLOEXEC;
#endif
        do { __fd=::open(__token.c_str(),__flags); } while (__fd<0 && errno==EINTR);
        if (__fd<0) __fail(errno);
#ifndef O_CLOEXEC
        if (::fcntl(__fd,F_SETFD,FD_CLOEXEC)<0) {
            int __error=errno; ::close(__fd); __fd=-1; __fail(__error);
        }
#endif
    }
    ~random_device() noexcept { int __saved=errno; if (__fd>=0) ::close(__fd); errno=__saved; }
    random_device(const random_device&)=delete;
    random_device& operator=(const random_device&)=delete;
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return ~0U; }
    double entropy() const noexcept { return 0; } // No portable entropy estimate.
    result_type operator()() {
        result_type __value; unsigned char* __p=reinterpret_cast<unsigned char*>(&__value);
        size_t __left=sizeof(__value);
        while (__left) {
            ssize_t __n=::read(__fd,__p,__left);
            if (__n<0 && errno==EINTR) continue;
            if (__n<=0) __fail(__n<0?errno:EIO);
            __p+=__n; __left-=size_t(__n);
        }
        return __value;
    }
};
template<class _Real=double> class exponential_distribution {
public:
    typedef _Real result_type;
    class param_type {
        _Real __lambda;
    public:
        typedef exponential_distribution distribution_type;
        explicit param_type(_Real __lambda=1) : __lambda(__lambda) {}
        _Real lambda() const { return __lambda; }
        friend bool operator==(const param_type& __x,const param_type& __y) { return __x.__lambda==__y.__lambda; }
        friend bool operator!=(const param_type& __x,const param_type& __y) { return !(__x==__y); }
    };
private:
    param_type __param;
public:
    explicit exponential_distribution(_Real __lambda=1) : __param(__lambda) {}
    explicit exponential_distribution(const param_type& __p) : __param(__p) {}
    void reset() {  }
    _Real lambda() const { return __param.lambda(); }
    _Real min() const { return 0; }
    _Real max() const { return numeric_limits<_Real>::infinity(); }
    param_type param() const { return __param; } void param(const param_type& __p) { __param=__p; }
    template<class _G> _Real operator()(_G& __g) { return (*this)(__g,__param); }
    template<class _G> _Real operator()(_G& __g,const param_type& __p) {
        return -std::log1p(-generate_canonical<_Real,numeric_limits<_Real>::digits>(__g))/__p.lambda();
    }
    friend bool operator==(const exponential_distribution& __x,const exponential_distribution& __y) { return __x.__param==__y.__param; }
    friend bool operator!=(const exponential_distribution& __x,const exponential_distribution& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __out,const exponential_distribution& __d) {
        __rw_random::__format_guard<basic_ostream<_C,_T> > __format(__out,ios_base::dec|ios_base::scientific|ios_base::left);
        __out.precision(2+numeric_limits<_Real>::digits*30103/100000);
        __out<<__d.lambda(); return __out;
    }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __in,exponential_distribution& __d) {
        __rw_random::__format_guard<basic_istream<_C,_T> > __format(__in,ios_base::dec|ios_base::skipws);
        exponential_distribution __copy=__d; _Real __lambda;
        if (!(__in>>__lambda)) return __in;
        __copy.param(param_type(__lambda)); __d=__copy; return __in;
    }
};
template<class _Real=double> class normal_distribution {
public:
    typedef _Real result_type;
    class param_type {
        _Real __mean,__stddev;
    public:
        typedef normal_distribution distribution_type;
        explicit param_type(_Real __mean=0,_Real __stddev=1) : __mean(__mean),__stddev(__stddev) {}
        _Real mean() const { return __mean; } _Real stddev() const { return __stddev; }
        friend bool operator==(const param_type& __x,const param_type& __y) { return __x.__mean==__y.__mean && __x.__stddev==__y.__stddev; }
        friend bool operator!=(const param_type& __x,const param_type& __y) { return !(__x==__y); }
    };
private:
    param_type __param; bool __has_spare=false; _Real __spare=0;
public:
    explicit normal_distribution(_Real __mean=0,_Real __stddev=1) : __param(__mean,__stddev) {}
    explicit normal_distribution(const param_type& __p) : __param(__p) {}
    void reset() { __has_spare=false; }
    _Real mean() const { return __param.mean(); } _Real stddev() const { return __param.stddev(); }
    _Real min() const { return -numeric_limits<_Real>::infinity(); }
    _Real max() const { return numeric_limits<_Real>::infinity(); }
    param_type param() const { return __param; } void param(const param_type& __p) { __param=__p; }
    template<class _G> _Real operator()(_G& __g) { return (*this)(__g,__param); }
    template<class _G> _Real operator()(_G& __g,const param_type& __p) {
        _Real __z;
        if (__has_spare) { __z=__spare; __has_spare=false; }
        else {
            _Real __x,__y,__r;
            do {
                __x=2*generate_canonical<_Real,numeric_limits<_Real>::digits>(__g)-1;
                __y=2*generate_canonical<_Real,numeric_limits<_Real>::digits>(__g)-1;
                __r=__x*__x+__y*__y;
            } while (__r>=1 || __r==0);
            _Real __scale=std::sqrt(-2*std::log(__r)/__r);
            __z=__x*__scale; __spare=__y*__scale; __has_spare=true;
        }
        return __p.mean()+__p.stddev()*__z;
    }
    friend bool operator==(const normal_distribution& __x,const normal_distribution& __y) { return __x.__param==__y.__param && __x.__has_spare==__y.__has_spare && (!__x.__has_spare || __x.__spare==__y.__spare); }
    friend bool operator!=(const normal_distribution& __x,const normal_distribution& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __out,const normal_distribution& __d) {
        __rw_random::__format_guard<basic_ostream<_C,_T> > __format(__out,ios_base::dec|ios_base::scientific|ios_base::left);
        __out.precision(2+numeric_limits<_Real>::digits*30103/100000);
        __out<<__d.mean(); __out<<__out.widen(' '); __out<<__d.stddev(); __out<<__out.widen(' ')<<__d.__has_spare; if (__d.__has_spare) __out<<__out.widen(' ')<<__d.__spare; return __out;
    }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __in,normal_distribution& __d) {
        __rw_random::__format_guard<basic_istream<_C,_T> > __format(__in,ios_base::dec|ios_base::skipws);
        normal_distribution __copy=__d; _Real __mean,__stddev;
        if (!(__in>>__mean>>__stddev>>__copy.__has_spare)) return __in; if (__copy.__has_spare && !(__in>>__copy.__spare)) return __in;
        __copy.param(param_type(__mean,__stddev)); __d=__copy; return __in;
    }
};
template<class _Real=double> class lognormal_distribution {
public:
    typedef _Real result_type;
    class param_type {
        _Real __m,__s;
    public:
        typedef lognormal_distribution distribution_type;
        explicit param_type(_Real __m=0,_Real __s=1) : __m(__m),__s(__s) {}
        _Real m() const { return __m; } _Real s() const { return __s; }
        friend bool operator==(const param_type& __x,const param_type& __y) { return __x.__m==__y.__m && __x.__s==__y.__s; }
        friend bool operator!=(const param_type& __x,const param_type& __y) { return !(__x==__y); }
    };
private:
    param_type __param; normal_distribution<_Real> __normal;
public:
    explicit lognormal_distribution(_Real __m=0,_Real __s=1) : __param(__m,__s) {}
    explicit lognormal_distribution(const param_type& __p) : __param(__p) {}
    void reset() { __normal.reset(); }
    _Real m() const { return __param.m(); } _Real s() const { return __param.s(); }
    _Real min() const { return 0; }
    _Real max() const { return numeric_limits<_Real>::infinity(); }
    param_type param() const { return __param; } void param(const param_type& __p) { __param=__p; }
    template<class _G> _Real operator()(_G& __g) { return (*this)(__g,__param); }
    template<class _G> _Real operator()(_G& __g,const param_type& __p) {
        return std::exp(__normal(__g,typename normal_distribution<_Real>::param_type(__p.m(),__p.s())));
    }
    friend bool operator==(const lognormal_distribution& __x,const lognormal_distribution& __y) { return __x.__param==__y.__param && __x.__normal==__y.__normal; }
    friend bool operator!=(const lognormal_distribution& __x,const lognormal_distribution& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __out,const lognormal_distribution& __d) {
        __rw_random::__format_guard<basic_ostream<_C,_T> > __format(__out,ios_base::dec|ios_base::scientific|ios_base::left);
        __out.precision(2+numeric_limits<_Real>::digits*30103/100000);
        __out<<__d.m(); __out<<__out.widen(' '); __out<<__d.s(); __out<<__out.widen(' ')<<__d.__normal; return __out;
    }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __in,lognormal_distribution& __d) {
        __rw_random::__format_guard<basic_istream<_C,_T> > __format(__in,ios_base::dec|ios_base::skipws);
        lognormal_distribution __copy=__d; _Real __m,__s;
        if (!(__in>>__m>>__s>>__copy.__normal)) return __in;
        __copy.param(param_type(__m,__s)); __d=__copy; return __in;
    }
};
namespace __rw_random {
// Marsaglia's cubic normal proposal for gamma shapes >= 1. Keep the full
// logarithmic acceptance test, without a polynomial squeeze approximation.
template<class _Real,class _G> _Real __gamma_shape(_Real __shape,_G& __g) {
    const _Real __d=__shape-_Real(1)/3,__c=1/std::sqrt(9*__d);
    normal_distribution<_Real> __normal;
    for (;;) {
        _Real __z=__normal(__g),__v=1+__c*__z;
        if (__v<=0) continue;
        __v=__v*__v*__v;
        _Real __u=generate_canonical<_Real,numeric_limits<_Real>::digits>(__g);
        if (std::log(__u)<_Real(0.5)*__z*__z+__d*(1-__v+std::log(__v))) return __d*__v;
    }
}
}
template<class _Real=double> class gamma_distribution {
    static_assert(is_floating_point<_Real>::value,"gamma distribution requires a floating-point result type");
public:
    typedef _Real result_type;
    class param_type {
        _Real __alpha,__beta;
    public:
        typedef gamma_distribution distribution_type;
        explicit param_type(_Real __alpha=1,_Real __beta=1) : __alpha(__alpha),__beta(__beta) {}
        _Real alpha() const { return __alpha; } _Real beta() const { return __beta; }
        friend bool operator==(const param_type& __x,const param_type& __y) { return __x.__alpha==__y.__alpha && __x.__beta==__y.__beta; }
        friend bool operator!=(const param_type& __x,const param_type& __y) { return !(__x==__y); }
    };
private: param_type __param;
public:
    explicit gamma_distribution(_Real __alpha=1,_Real __beta=1) : __param(__alpha,__beta) {}
    explicit gamma_distribution(const param_type& __p) : __param(__p) {}
    void reset() {}
    _Real alpha() const { return __param.alpha(); } _Real beta() const { return __param.beta(); }
    result_type min() const { return 0; }
    result_type max() const { return numeric_limits<_Real>::infinity(); }
    param_type param() const { return __param; } void param(const param_type& __p) { __param=__p; }
    template<class _G> result_type operator()(_G& __g) { return (*this)(__g,__param); }
    template<class _G> result_type operator()(_G& __g,const param_type& __p) {
        // Raising a uniform sample extends the shape >= 1 proposal to (0,1).
        _Real __shape=__p.alpha();
        _Real __value=__rw_random::__gamma_shape(__shape<1?__shape+1:__shape,__g);
        if (__shape<1) {
            _Real __u=generate_canonical<_Real,numeric_limits<_Real>::digits>(__g);
            __value*=std::pow(__u,1/__shape);
        }
        return __value*__p.beta();
    }
    friend bool operator==(const gamma_distribution& __x,const gamma_distribution& __y) { return __x.__param==__y.__param; }
    friend bool operator!=(const gamma_distribution& __x,const gamma_distribution& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __out,const gamma_distribution& __d) {
        __rw_random::__format_guard<basic_ostream<_C,_T> > __format(__out,ios_base::dec|ios_base::scientific|ios_base::left);
        __out.precision(2+numeric_limits<_Real>::digits*30103/100000);
        __out<<__d.alpha(); __out<<__out.widen(' '); __out<<__d.beta(); return __out;
    }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __in,gamma_distribution& __d) {
        __rw_random::__format_guard<basic_istream<_C,_T> > __format(__in,ios_base::dec|ios_base::skipws);
        _Real __alpha,__beta; if (__in>>__alpha>>__beta) __d.param(param_type(__alpha,__beta)); return __in;
    }
};
template<class _Int=int> class geometric_distribution {
    static_assert(is_integral<_Int>::value && !is_same<_Int,bool>::value,"integer distribution requires an integer result type");
public:
    typedef _Int result_type;
    class param_type {
        double __p;
    public:
        typedef geometric_distribution distribution_type;
        explicit param_type(double __p=0.5) : __p(__p) {}
        double p() const { return __p; }
        friend bool operator==(const param_type& __x,const param_type& __y) { return __x.__p==__y.__p; }
        friend bool operator!=(const param_type& __x,const param_type& __y) { return !(__x==__y); }
    };
private: param_type __param;
public:
    explicit geometric_distribution(double __p=0.5) : __param(__p) {}
    explicit geometric_distribution(const param_type& __p) : __param(__p) {}
    void reset() {}
    double p() const { return __param.p(); }
    result_type min() const { return 0; }
    result_type max() const { return (numeric_limits<_Int>::max)(); }
    param_type param() const { return __param; } void param(const param_type& __p) { __param=__p; }
    template<class _G> result_type operator()(_G& __g) { return (*this)(__g,__param); }
    template<class _G> result_type operator()(_G& __g,const param_type& __pr) {
        if (__pr.p()>=1) return 0;
        const double __denominator=std::log1p(-__pr.p());
        const long double __limit=static_cast<long double>((numeric_limits<_Int>::max)())+1;
        for (;;) {
            double __u=generate_canonical<double,numeric_limits<double>::digits>(__g);
            long double __failures=std::floor(std::log1p(-__u)/__denominator);
            if (__failures<__limit) return _Int(__failures);
        }
    }
    friend bool operator==(const geometric_distribution& __x,const geometric_distribution& __y) { return __x.__param==__y.__param; }
    friend bool operator!=(const geometric_distribution& __x,const geometric_distribution& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __out,const geometric_distribution& __d) {
        __rw_random::__format_guard<basic_ostream<_C,_T> > __format(__out,ios_base::dec|ios_base::scientific|ios_base::left);
        __out.precision(17); __out<<__d.p(); return __out;
    }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __in,geometric_distribution& __d) {
        __rw_random::__format_guard<basic_istream<_C,_T> > __format(__in,ios_base::dec|ios_base::skipws);
        double __p; if (__in>>__p) __d.param(param_type(__p)); return __in;
    }
};
template<class _Int=int> class binomial_distribution {
    static_assert(is_integral<_Int>::value && !is_same<_Int,bool>::value,"integer distribution requires an integer result type");
public:
    typedef _Int result_type;
    class param_type {
        _Int __t; double __p;
    public:
        typedef binomial_distribution distribution_type;
        explicit param_type(_Int __t=1,double __p=0.5) : __t(__t),__p(__p) {}
        _Int t() const { return __t; } double p() const { return __p; }
        friend bool operator==(const param_type& __x,const param_type& __y) { return __x.__t==__y.__t && __x.__p==__y.__p; }
        friend bool operator!=(const param_type& __x,const param_type& __y) { return !(__x==__y); }
    };
private: param_type __param;
public:
    explicit binomial_distribution(_Int __t=1,double __p=0.5) : __param(__t,__p) {}
    explicit binomial_distribution(const param_type& __p) : __param(__p) {}
    void reset() {}
    _Int t() const { return __param.t(); } double p() const { return __param.p(); }
    result_type min() const { return 0; }
    result_type max() const { return t(); }
    param_type param() const { return __param; } void param(const param_type& __p) { __param=__p; }
    template<class _G> result_type operator()(_G& __g) { return (*this)(__g,__param); }
    template<class _G> result_type operator()(_G& __g,const param_type& __pr) {
        if (__pr.p()<=0 || __pr.t()==0) return 0;
        if (__pr.p()>=1) return __pr.t();
        const bool __reflect=__pr.p()>0.5;
        double __p=__reflect?1-__pr.p():__pr.p();
        _Int __n=__pr.t(),__successes=0;
        // The a-th order statistic of n uniforms is Beta(a,n-a+1).
        // Conditioning on it halves the remaining number of trials.
        while (__n>64 && double(__n)*__p>16) {
            _Int __a=__n/2+1,__b=__n-__a+1;
            double __x=__rw_random::__gamma_shape(double(__a),__g);
            double __y=__rw_random::__gamma_shape(double(__b),__g);
            double __split=__x/(__x+__y);
            if (__split>=__p) { __n=__a-1; __p=__p/__split; }
            else { __successes+=__a; __n=__b-1; __p=(__p-__split)/(1-__split); }
            if (__p<=0) { __n=0; break; }
            if (__p>=1) { __successes+=__n; __n=0; break; }
        }
        if (__n) {
            const double __denominator=std::log1p(-__p);
            while (__n) {
                double __u=generate_canonical<double,numeric_limits<double>::digits>(__g);
                long double __gap=std::floor(std::log1p(-__u)/__denominator);
                if (__gap>=static_cast<long double>(__n)) break;
                __n-=_Int(__gap); --__n; ++__successes;
            }
        }
        return __reflect?_Int(__pr.t()-__successes):__successes;
    }
    friend bool operator==(const binomial_distribution& __x,const binomial_distribution& __y) { return __x.__param==__y.__param; }
    friend bool operator!=(const binomial_distribution& __x,const binomial_distribution& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __out,const binomial_distribution& __d) {
        __rw_random::__format_guard<basic_ostream<_C,_T> > __format(__out,ios_base::dec|ios_base::scientific|ios_base::left);
        __out.precision(17); __out<<__d.t(); __out<<__out.widen(' '); __out<<__d.p(); return __out;
    }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __in,binomial_distribution& __d) {
        __rw_random::__format_guard<basic_istream<_C,_T> > __format(__in,ios_base::dec|ios_base::skipws);
        _Int __t; double __p; if (__in>>__t>>__p) __d.param(param_type(__t,__p)); return __in;
    }
};
template<class _Int=int> class poisson_distribution {
    static_assert(is_integral<_Int>::value && !is_same<_Int,bool>::value,"integer distribution requires an integer result type");
public:
    typedef _Int result_type;
    class param_type {
        double __mean;
    public:
        typedef poisson_distribution distribution_type;
        explicit param_type(double __mean=1) : __mean(__mean) {}
        double mean() const { return __mean; }
        friend bool operator==(const param_type& __x,const param_type& __y) { return __x.__mean==__y.__mean; }
        friend bool operator!=(const param_type& __x,const param_type& __y) { return !(__x==__y); }
    };
private: param_type __param;
public:
    explicit poisson_distribution(double __mean=1) : __param(__mean) {}
    explicit poisson_distribution(const param_type& __p) : __param(__p) {}
    void reset() {}
    double mean() const { return __param.mean(); }
    result_type min() const { return 0; }
    result_type max() const { return (numeric_limits<_Int>::max)(); }
    param_type param() const { return __param; } void param(const param_type& __p) { __param=__p; }
    template<class _G> result_type operator()(_G& __g) { return (*this)(__g,__param); }
    template<class _G> result_type operator()(_G& __g,const param_type& __pr) {
        const _Int __maximum=(numeric_limits<_Int>::max)();
        for (;;) {
            _Int __count=0; double __remaining=__pr.mean(); bool __retry=false;
            // Conditional on the m-th arrival at y, earlier arrivals are
            // uniform on [0,y]. This reduces a large Poisson draw to gamma
            // and binomial draws rather than looping once per event.
            while (__remaining>=16) {
                double __candidate=std::floor(__remaining);
                _Int __m=static_cast<long double>(__candidate)>=static_cast<long double>(__maximum)?__maximum:_Int(__candidate);
                double __y=__rw_random::__gamma_shape(double(__m),__g);
                if (__y>=__remaining) {
                    _Int __part=binomial_distribution<_Int>(__m-1,__remaining/__y)(__g);
                    if (__part<=__maximum-__count) return _Int(__count+__part);
                    __retry=true; break;
                }
                if (__m>__maximum-__count) { __retry=true; break; }
                __count+=__m; __remaining-=__y;
            }
            if (__retry) continue;
            double __product=1,__limit=std::exp(-__remaining);
            for (;;) {
                __product*=generate_canonical<double,numeric_limits<double>::digits>(__g);
                if (__product<=__limit) return __count;
                if (__count==__maximum) break;
                ++__count;
            }
        }
    }
    friend bool operator==(const poisson_distribution& __x,const poisson_distribution& __y) { return __x.__param==__y.__param; }
    friend bool operator!=(const poisson_distribution& __x,const poisson_distribution& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __out,const poisson_distribution& __d) {
        __rw_random::__format_guard<basic_ostream<_C,_T> > __format(__out,ios_base::dec|ios_base::scientific|ios_base::left);
        __out.precision(17); __out<<__d.mean(); return __out;
    }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __in,poisson_distribution& __d) {
        __rw_random::__format_guard<basic_istream<_C,_T> > __format(__in,ios_base::dec|ios_base::skipws);
        double __mean; if (__in>>__mean) __d.param(param_type(__mean)); return __in;
    }
};
template<class _Int=int> class negative_binomial_distribution {
    static_assert(is_integral<_Int>::value && !is_same<_Int,bool>::value,"integer distribution requires an integer result type");
public:
    typedef _Int result_type;
    class param_type {
        _Int __k; double __p;
    public:
        typedef negative_binomial_distribution distribution_type;
        explicit param_type(_Int __k=1,double __p=0.5) : __k(__k),__p(__p) {}
        _Int k() const { return __k; } double p() const { return __p; }
        friend bool operator==(const param_type& __x,const param_type& __y) { return __x.__k==__y.__k && __x.__p==__y.__p; }
        friend bool operator!=(const param_type& __x,const param_type& __y) { return !(__x==__y); }
    };
private: param_type __param;
public:
    explicit negative_binomial_distribution(_Int __k=1,double __p=0.5) : __param(__k,__p) {}
    explicit negative_binomial_distribution(const param_type& __p) : __param(__p) {}
    void reset() {}
    _Int k() const { return __param.k(); } double p() const { return __param.p(); }
    result_type min() const { return 0; }
    result_type max() const { return (numeric_limits<_Int>::max)(); }
    param_type param() const { return __param; } void param(const param_type& __p) { __param=__p; }
    template<class _G> result_type operator()(_G& __g) { return (*this)(__g,__param); }
    template<class _G> result_type operator()(_G& __g,const param_type& __pr) {
        if (__pr.p()>=1) return 0;
        // A gamma mixture of Poisson rates counts failures before k successes.
        double __rate=gamma_distribution<double>(double(__pr.k()),(1-__pr.p())/__pr.p())(__g);
        if (__rate==0) return 0; // A very small rate may underflow.
        return poisson_distribution<_Int>(__rate)(__g);
    }
    friend bool operator==(const negative_binomial_distribution& __x,const negative_binomial_distribution& __y) { return __x.__param==__y.__param; }
    friend bool operator!=(const negative_binomial_distribution& __x,const negative_binomial_distribution& __y) { return !(__x==__y); }
    template<class _C,class _T> friend basic_ostream<_C,_T>& operator<<(basic_ostream<_C,_T>& __out,const negative_binomial_distribution& __d) {
        __rw_random::__format_guard<basic_ostream<_C,_T> > __format(__out,ios_base::dec|ios_base::scientific|ios_base::left);
        __out.precision(17); __out<<__d.k(); __out<<__out.widen(' '); __out<<__d.p(); return __out;
    }
    template<class _C,class _T> friend basic_istream<_C,_T>& operator>>(basic_istream<_C,_T>& __in,negative_binomial_distribution& __d) {
        __rw_random::__format_guard<basic_istream<_C,_T> > __format(__in,ios_base::dec|ios_base::skipws);
        _Int __k; double __p; if (__in>>__k>>__p) __d.param(param_type(__k,__p)); return __in;
    }
};
template<class _U,_U __a,_U __c,_U __m> constexpr _U linear_congruential_engine<_U,__a,__c,__m>::multiplier;
template<class _U,_U __a,_U __c,_U __m> constexpr _U linear_congruential_engine<_U,__a,__c,__m>::increment;
template<class _U,_U __a,_U __c,_U __m> constexpr _U linear_congruential_engine<_U,__a,__c,__m>::modulus;
template<class _U,_U __a,_U __c,_U __m> constexpr _U linear_congruential_engine<_U,__a,__c,__m>::default_seed;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr size_t mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::word_size;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr size_t mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::state_size;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr size_t mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::shift_size;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr size_t mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::mask_bits;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr size_t mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::tempering_u;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr size_t mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::tempering_s;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr size_t mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::tempering_t;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr size_t mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::tempering_l;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr _U mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::xor_mask;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr _U mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::tempering_d;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr _U mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::tempering_b;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr _U mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::tempering_c;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr _U mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::initialization_multiplier;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr _U mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::default_seed;
template<class _U,size_t __w,size_t __n,size_t __m,size_t __r,_U __a,size_t __u,_U __d,size_t __s,_U __b,size_t __t,_U __c,size_t __l,_U __f> constexpr _U mersenne_twister_engine<_U,__w,__n,__m,__r,__a,__u,__d,__s,__b,__t,__c,__l,__f>::__mask;
template<class _U,size_t __w,size_t __s,size_t __r> constexpr size_t subtract_with_carry_engine<_U,__w,__s,__r>::word_size;
template<class _U,size_t __w,size_t __s,size_t __r> constexpr size_t subtract_with_carry_engine<_U,__w,__s,__r>::short_lag;
template<class _U,size_t __w,size_t __s,size_t __r> constexpr size_t subtract_with_carry_engine<_U,__w,__s,__r>::long_lag;
template<class _U,size_t __w,size_t __s,size_t __r> constexpr _U subtract_with_carry_engine<_U,__w,__s,__r>::default_seed;
}
#endif
