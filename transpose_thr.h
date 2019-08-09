#ifndef NTHREADS
#define NTHREADS 1
#endif

typedef struct arg {
  DTYPE *A;
  DTYPE *B;
  long t;
} threadArg;

#if defined ROW_NAIVE
void *rowThreadedTranspose_naive(void *t);
#elif defined COL_NAIVE
void *colThreadedTranspose_naive(void *t);
#endif
