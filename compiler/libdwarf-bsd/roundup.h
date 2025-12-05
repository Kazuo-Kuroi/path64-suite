
#ifndef _ROUNDUP_H_
#define _ROUNDUP_H_

#include <sys/param.h>

#ifndef roundup
# define roundup(x, y)  ((((x) + ((y) - 1)) / (y)) * (y))
#endif // !defined(roundup)


#endif // _ROUNDUP_H_

