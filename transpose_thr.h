#ifndef NROWS
#define NROWS 8192
#endif

#ifndef NCOLS
#define NCOLS 8192
#endif

#ifndef NTHREADS
#define NTHREADS 1
#endif

struct argStruct {
  DTYPE *A;
  DTYPE *B;
  long t;
};

#if defined ROW_NAIVE
void *rowThreadedTranspose_naive(void *t);
#elif defined COL_NAIVE
void *colThreadedTranspose_naive(void *t);
#endif

#if defined BROWS && defined BCOLS
void blockedTranspose(DTYPE * restrict A, DTYPE * restrict B);
#endif
