
extern	double	__trunc(double);

#if defined(__GNUC__)
extern  long double  __truncl(long double);

long double    truncl() __attribute__ ((weak, alias ("__truncl")));

#endif

long double
__truncl(long double x)
{
	return ( (long double)__trunc((double)x) );
}

