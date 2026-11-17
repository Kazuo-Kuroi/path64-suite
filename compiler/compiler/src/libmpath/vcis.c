/*
 * Copyright (C) 2007. PathScale, LLC. All Rights Reserved.
 */
#include "complex.h"
#include "vector.h"

/* This needs to be defined for linux as well as maybe other platforms */
/*vector_declare(sincos);*/

#ifdef __x86_64__
#include "w_sincos.c"
#endif

dcomplex FN_PROTOTYPE(cis)(double x) __attribute__((always_inline));

dcomplex FN_PROTOTYPE(cis)(double x)
{
  dcomplex result;
  FN_NAME(sincos)(x, &result.dimag, &result.dreal);
  return result;
}

vector_function(cis, const double *x, dcomplex *y);
