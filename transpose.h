#if defined SERIAL
#if defined NAIVE
void naiveTranspose(DTYPE * restrict A, DTYPE * restrict B);
#elif defined BLOCKED
void blockedTranspose(DTYPE * restrict A, DTYPE * restrict B);
#elif defined INTRINSICS
void intrin8x8Transpose(double * restrict A, double * restrict B);
#endif
#elif defined THREADS
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
#endif
