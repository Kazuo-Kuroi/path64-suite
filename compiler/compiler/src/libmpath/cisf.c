/*
 * Copyright (C) 2007. PathScale, LLC. All Rights Reserved.
 */
#include <math.h>
#include "complex.h"
#include "vector.h"

#ifdef __x86_64__
#include "w_sincosf.c"
#endif

complex __libm_rcis(float x)
{
  complex result;
  FN_NAME(sincosf)(x, &result.imag, &result.real);
  return result;
}

complex __rcis(float x) __attribute__ ((weak, alias("__libm_rcis")));
