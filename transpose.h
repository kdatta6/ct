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

#if defined NAIVE_ROW
void *naiveRowThreadedTranspose(void *thrArg);
#elif defined NAIVE_COL
void *naiveColThreadedTranspose(void *thrArg);
#elif defined BLOCKED_ROW
void *blockedRowThreadedTranspose(void *thrArg);
#elif defined BLOCKED_COL
void *blockedColThreadedTranspose(void *thrArg);
#endif
#endif
