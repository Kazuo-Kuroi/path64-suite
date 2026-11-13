

#ifndef LIB_VERSION_H_INCLUDED
#define LIB_VERSION_H_INCLUDED


#include <math.h>


/* glibc before 2.27 has the SVID error interface in <math.h>: matherr,
 * struct exception, _LIB_VERSION and the DOMAIN, SING... macros. Newer
 * glibc removed it. DOMAIN is defined together with the rest, so use it
 * to tell them apart, and keep SVID behaviour where the library has it.
 */
#if defined(__linux) && defined(DOMAIN)

#define PATH64_LIB_VERSION_SVID (_LIB_VERSION == _SVID_)
#define PATH64_LIB_VERSION_POSIX (_LIB_VERSION == _POSIX_)

#elif defined(__linux) || defined(__FreeBSD__)

/* No SVID interface: report errors through errno only, as POSIX does. */
#define PATH64_LIB_VERSION_SVID 0
#define PATH64_LIB_VERSION_POSIX 1

struct exception {
    int type;
    char *name;
    double arg1;
    double arg2;
    double retval;
};

/* Types of exceptions in the `type' field.  */
# define DOMAIN         1
# define SING           2
# define OVERFLOW       3
# define UNDERFLOW      4
# define TLOSS          5
# define PLOSS          6

# ifndef HUGE
#  define HUGE          3.40282347e+38F
# endif

#define matherr(x) (1)

#elif defined(__NetBSD__)

#define PATH64_LIB_VERSION_SVID 0
#define PATH64_LIB_VERSION_POSIX 1

#elif defined(__sun)

#define PATH64_LIB_VERSION_SVID 0
#define PATH64_LIB_VERSION_POSIX 0

#else

#error "You should define lib version macros for your platform"

#endif

#endif // LIB_VERSION_H_INCLUDED

