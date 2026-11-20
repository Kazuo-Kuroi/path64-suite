// Original Path64 unbiased URBG sampling, shared with shuffle.
#ifndef _RWSTD_C11_RANDOM_BITS_INCLUDED
#define _RWSTD_C11_RANDOM_BITS_INCLUDED
namespace std {
namespace __rw_random {
// Rejection sampling extracts unbiased power-of-two chunks from any URBG.
// Unsigned wrap denotes a generator spanning all 64 bits.
template<class _G> unsigned long long __bits(_G& __g,unsigned __count) {
    typedef unsigned long long _U;
    _U __span=_U((_G::max)())-_U((_G::min)())+1;
    unsigned __width=0;
    if (!__span) __width=64;
    else { _U __n=__span; while (__n>1) { ++__width; __n>>=1; } }
    _U __result=0;
    while (__count) {
        _U __sample;
        do { __sample=_U(__g())-_U((_G::min)()); }
        while (__width!=64 && __sample>=(_U(1)<<__width));
        unsigned __take=__width<__count?__width:__count;
        if (__take==64) return __sample;
        __result=(__result<<__take)|(__sample&((_U(1)<<__take)-1));
        __count-=__take;
    }
    return __result;
}
template<class _G> unsigned long long __below(_G& __g,unsigned long long __span) {
    if (__span==1) return 0;
    unsigned __count=0;
    if (!__span) __count=64;
    else for (unsigned long long __n=__span-1;__n;__n>>=1) ++__count;
    unsigned long long __value;
    do { __value=__bits(__g,__count); } while (__span && __value>=__span);
    return __value;
}
}
}
#endif
