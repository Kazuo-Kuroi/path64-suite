
extern	double	__sqrt(double);

#if defined(__GNUC__)
extern  long double  __sqrtl(long double);

long double    sqrtl() __attribute__ ((weak, alias ("__sqrtl")));

#endif

long double
__sqrtl(long double x)
{
	return ( (long double)__sqrt((double)x) );
}

