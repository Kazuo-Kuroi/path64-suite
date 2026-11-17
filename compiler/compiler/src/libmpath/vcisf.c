/*
 * Copyright (C) 2007. PathScale, LLC. All Rights Reserved.
 */
#include "complex.h"
#include "vector.h"

/*vector_declare(sincosf);*/

#ifdef __x86_64__
#include "w_sincosf.c"
#endif

complex FN_PROTOTYPE(cisf)(float x) __attribute__((always_inline));

complex FN_PROTOTYPE(cisf)(float x)
{
  complex result;
  FN_NAME(sincosf)(x, &result.imag, &result.real);
  return result;
}

vector_function(cisf, const float *x, complex *y);
