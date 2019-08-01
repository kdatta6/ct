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

void *rowThreadedTranspose_naive(void *t);
void *colThreadedTranspose_naive(void *t);

#if defined BROWS && defined BCOLS
void blockedTranspose(double * restrict A, double * restrict B);
#endif
