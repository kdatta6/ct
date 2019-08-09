#ifndef NROWS
#define NROWS 8192
#endif

#ifndef NCOLS
#define NCOLS 8192
#endif

#if defined INTRINSICS
void intrin8x8Transpose(double * restrict A, double * restrict B);
#elif defined BROWS && defined BCOLS
void blockedTranspose(DTYPE * restrict A, DTYPE * restrict B);
#else
void transpose(DTYPE * restrict A, DTYPE * restrict B);
#endif
