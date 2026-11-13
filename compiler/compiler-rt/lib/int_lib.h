/* ===-- int_lib.h - configuration header for compiler-rt  -----------------===
 *
 *                     The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file is a configuration header for compiler-rt.
 * This file is not part of the interface of this library.
 *
 * ===----------------------------------------------------------------------===
 */

#ifndef INT_LIB_H
#define INT_LIB_H

/* Assumption:  signed integral is 2's complement */
/* Assumption:  right shift of signed negative is arithmetic shift */

#include <limits.h>
#include "endianness.h"
#include <math.h>

/* If compiling for kernel use, call panic() instead of abort(). */
#ifdef KERNEL_USE
extern void panic (const char *, ...);
#define compilerrt_abort() \
  panic("%s:%d: abort in %s", __FILE__, __LINE__, __FUNCTION__)
#else
#define compilerrt_abort() abort()
#endif

#if !defined(INFINITY) && defined(HUGE_VAL)
#define INFINITY HUGE_VAL
#endif /* INFINITY */

typedef      int si_int;
typedef unsigned su_int;

typedef          long long di_int;
typedef unsigned long long du_int;

typedef union
{
    di_int all;
    struct
    {
#if _YUGA_LITTLE_ENDIAN
        su_int low;
        si_int high;
#else
        si_int high;
        su_int low;
#endif /* _YUGA_LITTLE_ENDIAN */
    }s;
} dwords;

typedef union
{
    du_int all;
    struct
    {
#if _YUGA_LITTLE_ENDIAN
        su_int low;
        su_int high;
#else
        su_int high;
        su_int low;
#endif /* _YUGA_LITTLE_ENDIAN */
    }s;
} udwords;

/* 128-bit integer support. The PathScale compiler has no 128-bit
 * integer arithmetic: WHIRL lowers 16-byte integers to 64 bits, so it
 * rejects operations on mode (TI) values. Leave the TI routines out when
 * it builds this library; it never calls them.
 */
#if __x86_64 && !defined(__PATHCC__)
#define CRT_HAS_128BIT 1
#else
#define CRT_HAS_128BIT 0
#endif

#if CRT_HAS_128BIT

typedef int      ti_int __attribute__ ((mode (TI)));
typedef unsigned tu_int __attribute__ ((mode (TI)));

typedef union
{
    ti_int all;
    struct
    {
#if _YUGA_LITTLE_ENDIAN
        du_int low;
        di_int high;
#else
        di_int high;
        du_int low;
#endif /* _YUGA_LITTLE_ENDIAN */
    }s;
} twords;

typedef union
{
    tu_int all;
    struct
    {
#if _YUGA_LITTLE_ENDIAN
        du_int low;
        du_int high;
#else
        du_int high;
        du_int low;
#endif /* _YUGA_LITTLE_ENDIAN */
    }s;
} utwords;

static inline ti_int make_ti(di_int h, di_int l) {
    twords r;
    r.s.high = h;
    r.s.low = l;
    return r.all;
}

static inline tu_int make_tu(du_int h, du_int l) {
    utwords r;
    r.s.high = h;
    r.s.low = l;
    return r.all;
}

#endif /* CRT_HAS_128BIT */

typedef union
{
    su_int u;
    float f;
} float_bits;

typedef union
{
    udwords u;
    double  f;
} double_bits;

typedef struct
{
#if _YUGA_LITTLE_ENDIAN
    udwords low;
    udwords high;
#else
    udwords high;
    udwords low;
#endif /* _YUGA_LITTLE_ENDIAN */
} uqwords;

typedef union
{
    uqwords     u;
    long double f;
} long_double_bits;

#endif /* INT_LIB_H */
