
extern	double	__ceil(double);

#if defined(__GNUC__)
extern  long double  __ceill(long double);

long double    ceill() __attribute__ ((weak, alias ("__ceill")));

#endif

long double
__ceill(long double x)
{
	return ( (long double)__ceil((double)x) );
}

