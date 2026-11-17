/*
 * Copyright (C) 2007. PathScale, LLC. All Rights Reserved.
 */
#include <math.h>
#include "complex.h"
#include "vector.h"

#ifdef __x86_64__
#include "w_sincos.c"
#endif

dcomplex __libm_dcis(double x)
{
  dcomplex result;
  FN_NAME(sincos)(x, &result.dimag, &result.dreal);
  return result;
}

dcomplex __dcis(double x) __attribute__ ((weak, alias("__libm_dcis")));
