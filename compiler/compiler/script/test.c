#include <immintrin.h>

union{
	__m256i wi;
	int x[8];
  }v8si1,v8si2,v8si3;
int main(){
  union{
    __m256d wi;
    double x[4];
  }A,B,C;
  A.x[0] = B.x[0] = 1.1;
  A.x[1] = B.x[1] = 2.2;
  A.x[2] = B.x[2] = 13.2;
  A.x[3] = B.x[3] = 1.1;
  union{
    __m256 wi;
    float x[8];
  }AA,BB,D;
  AA.x[0] = BB.x[0] = 1.1;
  AA.x[1] = BB.x[1] = 2.2;
  AA.x[2] = BB.x[2] = 13.2;
  AA.x[3] = BB.x[3] = 1.1;
  AA.x[4] = BB.x[4] = 1.1;
  AA.x[5] = BB.x[5] = 2.2;
  AA.x[6] = BB.x[6] = 13.2;
  AA.x[7] = BB.x[7] = 1.1;

  union{
    __m128 wi;
    float x[4];
  }v4sf1,v4sf2,v4sf3;
  v4sf1.x[0] = v4sf2.x[0] = 1.1;
  v4sf1.x[1] = v4sf2.x[1] = 2.2;
  v4sf1.x[2] = v4sf2.x[2] = 13.2;
  v4sf1.x[3] = v4sf2.x[3] = 1.1;
  
  union{
    __m256i wi;
    long long x[4];
  }v4di1,v4di2,v4di3;
  v4di1.x[0] = v4di2.x[0] = 11111111;
  v4di1.x[1] = v4di2.x[1] = 2222222222222;
  v4di1.x[2] = v4di2.x[2] = 44444444444444;
  v4di1.x[3] = v4di2.x[3] = 123423423412341234;
  
  printf("%f %f %f %f ", AA.x[0], AA.x[1], AA.x[2], AA.x[3]);
  printf("%f %f %f %f \n", AA.x[4], AA.x[5], AA.x[6], AA.x[7]);
  printf("%f %f %f %f ", BB.x[0], BB.x[1], BB.x[2], BB.x[3]);
  printf("%f %f %f %f \n", BB.x[4], BB.x[5], BB.x[6], BB.x[7]);
 //AVX test
  printf("%lf %lf %lf %lf \n",A.x[0], A.x[1], A.x[2], A.x[3]);
  printf("%lf %lf %lf %lf \n",B.x[0], B.x[1], B.x[2], B.x[3]);
  C.wi = _mm256_add_pd(A.wi, B.wi);
  D.wi = _mm256_add_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
#if 1 
  C.wi = _mm256_addsub_pd(A.wi, B.wi);
  D.wi = _mm256_addsub_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);

  C.wi = _mm256_and_pd(A.wi, B.wi);
  D.wi = _mm256_and_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  
  C.wi = _mm256_andnot_pd(A.wi, B.wi);
  D.wi = _mm256_andnot_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  
  C.wi = _mm256_div_pd (A.wi, B.wi);
  D.wi = _mm256_div_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  
  C.wi = _mm256_hadd_pd (A.wi, B.wi);
  D.wi = _mm256_hadd_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  
  C.wi = _mm256_hsub_pd (A.wi, B.wi);
  D.wi = _mm256_hsub_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  
  C.wi = _mm256_max_pd (A.wi, B.wi);
  D.wi = _mm256_max_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  
  C.wi = _mm256_min_pd (A.wi, B.wi);
  D.wi = _mm256_min_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  
  C.wi = _mm256_mul_pd (A.wi, B.wi);
  D.wi = _mm256_mul_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  
  C.wi = _mm256_or_pd (A.wi, B.wi);
  D.wi = _mm256_or_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  
  C.wi = _mm256_sub_pd (A.wi, B.wi);
  D.wi = _mm256_sub_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  
  C.wi = _mm256_xor_pd (A.wi, B.wi);
  D.wi = _mm256_xor_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  
  C.wi = _mm256_unpackhi_pd (A.wi, B.wi);
  D.wi = _mm256_unpackhi_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);

  C.wi = _mm256_unpacklo_pd (A.wi, B.wi);
  D.wi = _mm256_unpacklo_ps (AA.wi, BB.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
#endif


  printf("\n----------below if for float=float op float op simm-----------\n");
  C.wi = _mm256_blend_pd (A.wi, B.wi, 2);
  D.wi = _mm256_blend_ps (AA.wi, BB.wi,2);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);

  D.wi=_mm256_dp_ps (AA.wi, BB.wi, 2);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[5], D.x[6], D.x[7], D.x[7]);

  C.wi = _mm256_shuffle_pd (A.wi, B.wi, 2);
  D.wi = _mm256_shuffle_ps (AA.wi, BB.wi,2);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);

  C.wi = _mm256_cmp_pd (A.wi, B.wi, 2);
  D.wi = _mm256_cmp_ps (AA.wi, BB.wi,2);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
//TODO _mm_cmp_pd/ps _mm_cmp_sd/ss

  
  C.wi = _mm256_permute2f128_pd (A.wi, B.wi, 2);
  D.wi = _mm256_permute2f128_ps (AA.wi, BB.wi,2);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  //TODO _mm256_permute2f128_si256

  //const int tconst=2;

  C.wi = _mm256_sqrt_pd (A.wi);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  
  v4sf3.wi = _mm_permute_ps(v4sf1.wi, 2);
  printf("%f %f %f %f \n",v4sf3.x[0], v4sf3.x[1], v4sf3.x[2], v4sf3.x[3]);
 
  //TODO zero
  //_mm256_zeroall();

  union{
	__m128i wi;
	int x[4];
  }v4si1,v4si2,v4si3;
  v4si1.x[0]=v4si2.x[0]=10;
  v4si1.x[1]=v4si2.x[1]=11;
  v4si1.x[2]=v4si2.x[2]=12;
  v4si1.x[3]=v4si2.x[3]=13;
#if 0 
  union{
	__m128 wi;
	float x[4];
  }v4sf1,v4sf2,v4sf3;
  v4sf1.x[0]=v4sf2.x[0]=10.1;
  v4sf1.x[1]=v4sf2.x[1]=11.33;
  v4sf1.x[2]=v4sf2.x[2]=12.44;
  v4sf1.x[3]=v4sf2.x[3]=13.55;
#endif
  v8si1.x[0]=v8si2.x[0]=10;
  v8si1.x[1]=v8si2.x[1]=11;
  v8si1.x[2]=v8si2.x[2]=12;
  v8si1.x[3]=v8si2.x[3]=13;
  v8si1.x[4]=v8si2.x[4]=101;
  v8si1.x[5]=v8si2.x[5]=111;
  v8si1.x[6]=v8si2.x[6]=122;
  v8si1.x[7]=v8si2.x[7]=132;
  __m256i *pm256i;
  pm256i = &(v8si1.wi);
  
  union{
	__m128d wi;
	double x[2];
  }v2df1,v2df2,v2df3;
  v2df1.x[0]=v2df2.x[0]=10.1;
  v2df1.x[1]=v2df2.x[1]=11.1;

  union{
	__m128i wi;
	long long x[2];
  }v2di1,v2di2,v2di3;
  v2di1.x[0]=v2di2.x[0]=10;
  v2di1.x[1]=v2di2.x[1]=11;

  v4si3.wi = _mm256_cvttpd_epi32(A.wi);
  printf("%d %d %d %d \n",v4si3.x[0], v4si3.x[1], v4si3.x[2], v4si3.x[3]);
  
  //v8si3.wi = _mm256_cvttps_epi32(AA.wi);//TODO with bug -O0 
  //printf("%d %d %d %d ", v8si3.x[0], v8si3.x[1], v8si3.x[2], v8si3.x[3]);
  //printf("%d %d %d %d \n", v8si3.x[4], v8si3.x[5], v8si3.x[6], v8si3.x[7]);
   float fl1,*pfloat;
   fl1 = 2.234;
   pfloat = &fl1;


  _mm_maskstore_ps (&fl1, v4si1.wi, v4sf2.wi);
  printf("%f\n",fl1);
  //_mm_maskstore_ps (float *__P, __m128 __M, __m128 __A)

#if 0
  C.wi = _mm256_insertf128_pd (A.wi, B.wi, 2);
  D.wi = _mm256_insertf128_ps (AA.wi, BB.wi,2);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  //TODO _mm256_insertf128_si256
#endif

  D.wi = _mm256_loadu_ps (pfloat);
  printf("%lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  printf("%f \n", D.x[0]);

  int reint;
  reint = _mm_testc_pd (v2df1.wi, v2df2.wi);
  printf("INTRN_VTESTPD128C %d\n", reint);

  reint = _mm256_testz_ps (AA.wi, BB.wi);
  printf("VTESTPS256Z %d\n", reint);

  v2df3.wi=_mm256_extractf128_pd (A.wi, 1);
  printf("INTRN_VEXTRACTF128D %lf %lf\n",v2df3.x[0],v2df3.x[1]);

  v2df3.wi = _mm_permute_pd (v2df1.wi, 2);
  printf("INTRN_VPERMILPD128 %lf %lf\n",v2df3.x[0],v2df3.x[1]);
  
  v8si3.wi = _mm256_loadu_si256 (pm256i);
  printf("INTRN_VMOVDQA256 %d %d %d %d ", v8si3.x[0], v8si3.x[1], v8si3.x[2], v8si3.x[3]);
  printf("%d %d %d %d \n", v8si3.x[4], v8si3.x[5], v8si3.x[6], v8si3.x[7]);

   v4si3.wi = _mm256_cvtpd_epi32 (A.wi);
   printf("INTRN_VCVTPD2DQ256 %d %d %d %d \n",v4si3.x[0], v4si3.x[1], v4si3.x[2], v4si3.x[3]);
   
   D.wi = _mm256_rcp_ps (AA.wi);
  printf("INTRN_VRCPPS256  %f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);

   double db1, *pdouble;
   db1 = 1.2345;
   pdouble = &db1;
   C.wi = _mm256_maskload_pd (pdouble, v4di1.wi);
   printf("INTRN_VMASKMOVPD256 %lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);

   
   reint = _mm256_testc_ps (AA.wi, BB.wi);
   printf("INTRN_VTESTPS256C %d\n", reint);


  _mm_maskstore_pd (pdouble, v2di1.wi, v2df2.wi);
  printf("INTRN_VMASKMOVPD128ST *pdouble=%f\n",*pdouble);

  C.wi = _mm256_permute_pd (A.wi, 4);
  printf("INTRN_VPERMILPD256 %lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
   

   v2df3.wi = _mm_maskload_pd (pdouble, v2di1.wi);
  printf("INTRN_VMASKMOVPD128 %lf %lf\n",v2df3.x[0],v2df3.x[1]);
  
   __m256i *pv8si = &(v8si3.wi);
   _mm256_storeu_si256 (pv8si, v8si1.wi);
   printf("INTRN_VMOVDQA256ST %d %d %d %d ", v8si3.x[0], v8si3.x[1], v8si3.x[2], v8si3.x[3]);
  printf("%d %d %d %d \n", v8si3.x[4], v8si3.x[5], v8si3.x[6], v8si3.x[7]);

   C.wi = _mm256_cvtepi32_pd (v4si1.wi);
   printf("INTRN_VCVTDQ2PD256 %lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);

  D.wi = _mm256_moveldup_ps (AA.wi);
  printf("INTRN_VMOVSLDUP256 %f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);


   D.wi = _mm256_permute_ps (AA.wi, 4);
  printf("INTRN_VPERMILPS256 %f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);

  v4sf3.wi = _mm256_cvtpd_ps (A.wi);
   printf("INTRN_VCVTPD2PS256 %f %f %f %f \n",v4sf3.x[0], v4sf3.x[1], v4sf3.x[2], v4sf3.x[3]);

  C.wi = _mm256_round_pd (A.wi, 2);
   printf("INTRN_VROUNDPD256 %lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);

  D.wi = _mm256_round_ps (AA.wi, 2);
  printf("INTRN_VROUNDPS256 %f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  
  v2df3.wi = _mm_permutevar_pd (v2df1.wi, v4si1.wi);
  printf("INTRN_VPERMILPD128VAR %lf %lf\n",v2df3.x[0],v2df3.x[1]);
  
  _mm256_maskstore_pd (pdouble, v4di1.wi, B.wi);
  printf("INTRN_VMASKMOVPD256ST pdouble=%lf\n",*pdouble);


  reint = _mm256_testnzc_pd (A.wi, B.wi);
   printf("INTRN_VTESTPD256NZC: %d\n", reint);

  v8si3.wi = _mm256_cvtps_epi32 (AA.wi);
   printf("INTRN_VCVTPS2DQ256: %d %d %d %d ", v8si3.x[0], v8si3.x[1], v8si3.x[2], v8si3.x[3]);
  printf("%d %d %d %d \n", v8si3.x[4], v8si3.x[5], v8si3.x[6], v8si3.x[7]);

  D.wi = _mm256_rsqrt_ps (AA.wi);
  printf("INTRN_VRSQRTPS256: %f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
  
  __m128 *pv4sf;
  pv4sf = &(v4sf1.wi);
  D.wi = _mm256_broadcast_ps (pv4sf);
  printf("INTRN_VBROADCAST128S: %f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);

  reint = _mm256_testc_pd (A.wi, B.wi);
   printf("INTRN_VTESTPD256C: %d\n", reint);


   v4sf3.wi = _mm_maskload_ps (pfloat, v4si1.wi);
   printf("INTRN_VMASKMOVPS128: %f %f %f %f \n",v4sf3.x[0], v4sf3.x[1], v4sf3.x[2], v4sf3.x[3]);

   D.wi = _mm256_broadcast_ss (pfloat);
  printf("INTRN_VBROADCASTSS256: %f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);

  D.wi = _mm256_movehdup_ps (AA.wi);
  printf("INTRN_VMOVSHDUP256: %f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);
	  
  D.wi = _mm256_sqrt_ps (AA.wi);
  printf("INTRN_VSQRTPS256: %f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);

  reint = _mm_testnzc_ps (v4sf1.wi, v4sf2.wi);
   printf("INTRN_VTESTPS128NZC: %d\n", reint);
  
   reint = _mm_testnzc_pd (v2df1.wi, v2df2.wi);
   printf("INTRN_VTESTPD128NZC: %d\n", reint);


  v4sf3.wi = _mm256_extractf128_ps (AA.wi, 1);
   printf("INTRN_VEXTRACTF128S:: %f %f %f %f \n",v4sf3.x[0], v4sf3.x[1], v4sf3.x[2], v4sf3.x[3]);


   C.wi = _mm256_permutevar_pd (A.wi, v4di1.wi);
   printf("INTRN_VPERMILPD256VAR:: %lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);

  //TODO INTRN_VZEROUPPER256
   
   __m128d *pv2df = &(v2df1.wi);
   C.wi = _mm256_broadcast_pd (pv2df);
   printf("INTRN_VBROADCAST128D: %lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);

  reint = _mm256_movemask_pd (A.wi);
  printf("INTRN_VMOVMSKPD256: %d\n", reint);

  _mm256_maskstore_ps (pfloat, v8si1.wi, BB.wi);
  printf("INTRN_VMASKMOVPS256: *pfloat =  %f\n", *pfloat);

  reint = _mm256_movemask_ps (AA.wi);
  printf("INTRN_VMOVMSKPS256: %d\n", reint);



  v8si3.wi = _mm256_lddqu_si256 (pv8si);
   printf("INTRN_VLDDQU256: %d %d %d %d ", v8si3.x[0], v8si3.x[1], v8si3.x[2], v8si3.x[3]);
  printf("%d %d %d %d \n", v8si3.x[4], v8si3.x[5], v8si3.x[6], v8si3.x[7]);

  reint = _mm_testc_ps (v4sf1.wi, v4sf1.wi);
  printf("INTRN_VTESTPS128C: %d\n", reint);


  v4sf3.wi = _mm_broadcast_ss (pfloat);
   printf("INTRN_VROADCASTSS128: %f %f %f %f \n",v4sf3.x[0], v4sf3.x[1], v4sf3.x[2], v4sf3.x[3]);

   _mm256_stream_si256 (pv8si, v8si2.wi);
   printf("INTRN_VMOVNTDQ256: %d %d %d %d ", v8si3.x[0], v8si3.x[1], v8si3.x[2], v8si3.x[3]);
  printf("%d %d %d %d \n", v8si3.x[4], v8si3.x[5], v8si3.x[6], v8si3.x[7]);

  D.wi = _mm256_permutevar_ps (AA.wi, v8si1.wi);
  printf("INTRN_VPERMILPS256VAR: %f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);

  v4si3.wi=_mm256_extractf128_si256 (v8si1.wi , 1);
   printf("INTRN_VEXTRACTF128I: %d %d %d %d \n",v4si3.x[0], v4si3.x[1], v4si3.x[2], v4si3.x[3]);

   D.wi = _mm256_cvtepi32_ps (v8si1.wi);
  printf("INTRN_VCVTDQ2PS256: %f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);

  v4sf3.wi = _mm_permutevar_ps (v4sf1.wi, v4si1.wi);
   printf("INTRN_VPERMILPS128VAR: %f %f %f %f \n",v4sf3.x[0], v4sf3.x[1], v4sf3.x[2], v4sf3.x[3]);

   _mm256_maskstore_ps (pfloat, v8si1.wi, BB.wi);
   printf("INTRN_VMASKMOVPS256ST: %f\n",*pfloat);

  reint = _mm256_testz_pd (A.wi, B.wi);
  printf("INTRN_VTESTPD256: %d\n", reint);

  reint = _mm256_testnzc_ps (AA.wi, BB.wi);
  printf("INTRN_VTESTPS256NZC: %d\n", reint);

  reint = _mm_testz_pd (v2df1.wi, v2df2.wi);
  printf("INTRN_VTESTPD128: %d\n", reint);

  C.wi = _mm256_broadcast_sd (pdouble);
   printf("INTRN_VBROADCASTSD256: %lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);

  C.wi = _mm256_cvtps_pd (v4sf1.wi);
   printf("INTRN_VCVTPS2PD256: %lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);

  
  C.wi = _mm256_movedup_pd (A.wi);
   printf("INTRN_VMOVDDUP256: %lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
 
   reint = _mm_testz_ps (v4sf1.wi, v4sf2.wi);
  printf("INTRN_VTESTPS128Z: %d\n", reint);
 
  v4si3.wi = _mm256_castsi256_si128 (v8si1.wi);
   printf("INTRN_VCASTSI256TOSI128: %d %d %d %d \n",v4si3.x[0], v4si3.x[1], v4si3.x[2], v4si3.x[3]);
 
   
	 v8si3.wi = _mm256_castsi128_si256 (v4si1.wi);
   printf("INTRN_VCASTSI128TOSI256 %d %d %d %d ", v8si3.x[0], v8si3.x[1], v8si3.x[2], v8si3.x[3]);
  printf("%d %d %d %d \n", v8si3.x[4], v8si3.x[5], v8si3.x[6], v8si3.x[7]);
   v4sf3.wi = _mm256_castps256_ps128 (AA.wi);
   printf("INTRN_VCASTPS256TOPS128 %f %f %f %f \n",v4sf3.x[0], v4sf3.x[1], v4sf3.x[2], v4sf3.x[3]);
   v2df3.wi=_mm256_castpd256_pd128 (A.wi);
  printf("INTRN_VCASTPD256TOPD128 %lf %lf\n",v2df3.x[0],v2df3.x[1]);
  D.wi = _mm256_castps128_ps256 (v4sf1.wi);
  printf("INTRN_VCASTPS128TOPS256: %f %f %f %f ", D.x[0], D.x[1], D.x[2], D.x[3]);
  _mm256_zeroupper ();
  printf("%f %f %f %f \n", D.x[4], D.x[5], D.x[6], D.x[7]);

  C.wi = _mm256_castpd128_pd256 (v2df1.wi);
   printf("INTRN_VCASTPD128TOPD256: %lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
  _mm256_zeroall ();
   printf("INTRN_VCASTPD128TOPD256: %lf %lf %lf %lf \n",C.x[0], C.x[1], C.x[2], C.x[3]);
 #if 0//this to the end
#endif
}
