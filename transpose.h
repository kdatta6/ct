#if defined SERIAL
#if defined NAIVE
void naiveTranspose(DTYPE * restrict A, DTYPE * restrict B);
#elif defined BLOCKED
void blockedTranspose(DTYPE * restrict A, DTYPE * restrict B);
#elif ((defined INTRINSICS_noSS) || (defined INTRINSICS_SS))
void intrin8x8Transpose(double * restrict A, double * restrict B);
#endif
#elif defined THREADS
typedef struct arg {
  DTYPE *A;
  DTYPE *B;
  long t;
} threadArg;

void *threadedTranspose(void *thrArg);
#endif
