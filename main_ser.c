#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "util.h"
#include "transpose_ser.h"

int main(int argc, char **argv) {
  DTYPE *A, *B;
#if defined CLEAR_L3_CACHE
  DTYPE *C;
#endif
  struct timeval t1, t2;
  double elapsedTime;
#if defined CHECK_ARRAY
  int rc;
#endif

  // allocate arrays
#if defined INTRINSICS
  // make sure arrays are allocated on 64 B boundaries
  A = (DTYPE *)aligned_alloc(64, NROWS * NCOLS * sizeof(DTYPE));
  B = (DTYPE *)aligned_alloc(64, NCOLS * NROWS * sizeof(DTYPE));
#else
  A = malloc(NROWS * NCOLS * sizeof(DTYPE));
  B = malloc(NCOLS * NROWS * sizeof(DTYPE));
#endif

  // init A
  initArray(A, NROWS, NCOLS);
  // zero out B
  zeroArray(B, NCOLS, NROWS);

#if defined PRINT_ARRAYS
  printf("A array:\n");
  printArray(A, NROWS, NCOLS);
#endif

#if defined CLEAR_L3_CACHE
  C = malloc(20000 * 20000 *sizeof(DTYPE));

  initArray(C, 20000, 20000);
#endif

  // start timer
  gettimeofday(&t1, NULL);

  // transpose
#if defined INTRINSICS
  intrin8x8Transpose(A, B);
#elif defined BROWS && defined BCOLS
  blockedTranspose(A, B);
#else
  transpose(A, B);
#endif

  // stop timer
  gettimeofday(&t2, NULL);

  // compute and print the elapsed time in millisec
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
  printf("%f ms.\n", elapsedTime);

#if defined PRINT_ARRAYS
  printf("B array:\n");
  printArray(B, NCOLS, NROWS);
#endif

#if defined CHECK_ARRAY
  rc = checkArray(A, B, NROWS, NCOLS);
  if (rc) {
    printf("TRANSPOSE FAILED\n");
  }
  else {
    printf("TRANSPOSE SUCCEEDED\n");
  }
#endif

  free(A);
  free(B);
#if defined CLEAR_L3_CACHE
  free(C);
#endif
}
