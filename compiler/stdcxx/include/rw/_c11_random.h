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
