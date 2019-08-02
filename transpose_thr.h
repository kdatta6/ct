#ifndef NROWS
#define NROWS 8192
#endif

#ifndef NCOLS
#define NCOLS 8192
#endif

#ifndef NTHREADS
#define NTHREADS 1
#endif

double* restrict A;
double* restrict B;

#if defined ROW_NAIVE
void *rowThreadedTranspose_naive(void *t);
#endif
#if defined COL_NAIVE
void *colThreadedTranspose_naive(void *t);
#endif

#if defined BROWS && defined BCOLS
void blockedTranspose(double * restrict A, double * restrict B);
#endif
