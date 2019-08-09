#if defined NAIVE
void naiveTranspose(DTYPE * restrict A, DTYPE * restrict B);
#elif defined BLOCKED
void blockedTranspose(DTYPE * restrict A, DTYPE * restrict B);
#elif defined INTRINSICS
void intrin8x8Transpose(double * restrict A, double * restrict B);
#endif
