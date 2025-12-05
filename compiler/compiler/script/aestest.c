//#include <wmmintrin.h>
typedef double __v2df __attribute__ ((__vector_size__ (16)));
typedef long long __v2di __attribute__ ((__vector_size__ (16)));
typedef int __v4si __attribute__ ((__vector_size__ (16)));
typedef short __v8hi __attribute__ ((__vector_size__ (16)));
typedef char __v16qi __attribute__ ((__vector_size__ (16)));

/* The Intel API is flexible enough that we must allow aliasing with other
   vector types, and their scalar components.  */
typedef long long __m128i __attribute__ ((__vector_size__ (16), __may_alias__));
typedef double __m128d __attribute__ ((__vector_size__ (16), __may_alias__));



extern __inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_aesdec_si128 (__m128i __X, __m128i __Y)
{
  return (__m128i) __builtin_ia32_aesdec128 ((__v2di)__X, (__v2di)__Y);
}

/* Performs the last round of AES decryption of the first m128i 
   using the second m128i as a round key.  */
extern __inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_aesdeclast_si128 (__m128i __X, __m128i __Y)
{
  return (__m128i) __builtin_ia32_aesdeclast128 ((__v2di)__X,(__v2di)__Y);
}

/* Performs 1 round of AES encryption of the first m128i using 
   the second m128i as a round key.  */
extern __inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_aesenc_si128 (__m128i __X, __m128i __Y)
{
  return (__m128i) __builtin_ia32_aesenc128 ((__v2di)__X, (__v2di)__Y);
}

/* Performs the last round of AES encryption of the first m128i
   using the second m128i as a round key.  */
extern __inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_aesenclast_si128 (__m128i __X, __m128i __Y)
{
  return (__m128i) __builtin_ia32_aesenclast128 ((__v2di)__X, (__v2di)__Y);
}

/* Performs the InverseMixColumn operation on the source m128i 
   and stores the result into m128i destination.  */
extern __inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_aesimc_si128 (__m128i __X)
{
  return (__m128i) __builtin_ia32_aesimc128 ((__v2di)__X);
}
extern __inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_aeskeygenassist_si128 (__m128i __X, const int __C)
{
  return (__m128i) __builtin_ia32_aeskeygenassist128 ((__v2di)__X, __C);
}
extern __inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_clmulepi64_si128 (__m128i __X, __m128i __Y, const int __I)
{
  return (__m128i) __builtin_ia32_pclmulqdq128 ((__v2di)__X,
						(__v2di)__Y, __I);
}

int main(){
  union {__m128i D; long long lli[2];}i128,i2,i3;
		long long w,e;
		i128.lli[0] = 0x123451234512345;
		i128.lli[1] = 0x555555555555555;
		i2.lli[0] =   0x222222222222222;
		i2.lli[1] =   0x333333333333333;

		i3.D = _mm_aesdec_si128(i128.D, i2.D);
		printf("%lx, %lx\n", i3.lli[0], i3.lli[1]);
		i3.D = _mm_aesdeclast_si128(i128.D, i2.D);
		printf("%lx, %lx\n", i3.lli[0], i3.lli[1]);
		i3.D = _mm_aesenc_si128(i128.D, i2.D);
		printf("%lx, %lx\n", i3.lli[0], i3.lli[1]);
		i3.D = _mm_aesdeclast_si128(i128.D, i2.D);
		printf("%lx, %lx\n", i3.lli[0], i3.lli[1]);
		i3.D = _mm_aesenclast_si128(i128.D, i2.D);
		printf("%lx, %lx\n", i3.lli[0], i3.lli[1]);
		i3.D = _mm_aesimc_si128(i128.D);
		printf("%lx, %lx\n", i3.lli[0], i3.lli[1]);
		i3.D = _mm_aeskeygenassist_si128(i128.D, 0x1);
		printf("%lx, %lx\n", i3.lli[0], i3.lli[1]);
		i3.D = _mm_clmulepi64_si128(i128.D, i2.D, 0x1);
		printf("%lx, %lx\n", i3.lli[0], i3.lli[1]);

		return 0;

}
