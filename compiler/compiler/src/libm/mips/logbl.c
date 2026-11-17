
extern	double	__logb(double);

#if defined(__GNUC__)
extern  long double  __logbl(long double);

long double    logbl() __attribute__ ((weak, alias ("__logbl")));

#endif

long double
__logbl(long double x)
{
	return ( (long double)__logb((double)x) );
}

