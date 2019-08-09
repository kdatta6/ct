#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "transpose_ser.h"

//#define CLEAR_L3_CACHE
//#define PRINT_ARRAYS

void initArray(DTYPE * restrict C, int nrows, int ncols) {
  int i, j;
  for (i = 0; i < nrows; i++) {
    for (j = 0; j < ncols; j++) {
      C[i*ncols + j] = i*ncols + j;
    }
  }
}

void zeroArray(DTYPE * restrict C, int nrows, int ncols) {
  int i, j;
  for (i = 0; i < nrows; i++) {
    for (j = 0; j < ncols; j++) {
      C[i*ncols + j] = 0;
    }
  }
}

#if defined PRINT_ARRAYS
void printArray(DTYPE * restrict C, int nrows, int ncols) {
  int i, j;
  for (i = 0; i < nrows; i++) {
    for (j = 0; j < ncols; j++) {
      printf("%f ", C[i*ncols + j]);
    }
    printf("\n");
  }
}
#endif

#if defined CHECK_ARRAY
// returns 1 if D is not a matrix of C, 0 otherwise
int checkArray(DTYPE * restrict C, DTYPE * restrict D, int nrows, int ncols) {
  int i, j;
  for (i = 0; i < nrows; i++) {
    for (j = 0; j < ncols; j++) {
      if (D[j*nrows + i] != C[i*ncols + j]) {
        return 1;
      }
    }
  }
  return 0;
}
#endif

int main(int argc, char **argv) {
  DTYPE *A, *B;
#if defined CLEAR_L3_CACHE
  DTYPE *C;
#endif
  struct timeval t1, t2;
  double elapsedTime;
  int rc;

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
