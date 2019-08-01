#include <stdio.h>
#include <stdlib.h>

#include "transpose_ser.h"

//#define CLEAR_L3_CACHE
//#define PRINT_ARRAYS

void initArray(double * restrict C, int nrows, int ncols) {
  int i, j;
  for (i = 0; i < nrows; i++) {
    for (j = 0; j < ncols; j++) {
      C[i*ncols + j] = i*ncols + j;
    }
  }
}

void zeroArray(double * restrict C, int nrows, int ncols) {
  int i, j;
  for (i = 0; i < nrows; i++) {
    for (j = 0; j < ncols; j++) {
      C[i*ncols + j] = 0;
    }
  }
}

#if defined PRINT_ARRAYS
void printArray(double * restrict C, int nrows, int ncols) {
  int i, j;
  for (i = 0; i < nrows; i++) {
    for (j = 0; j < ncols; j++) {
      printf("%f ", C[i*ncols + j]);
    }
    printf("\n");
  }
}
#endif

int main(int argc, char **argv) {
  double *A, *B;
#if defined CLEAR_L3_CACHE
  double *C;
#endif

  // allocate arrays
#if defined INTRINSICS
  // make sure arrays are allocated on 64 B boundaries
  A = (double *)aligned_alloc(64, NROWS * NCOLS * sizeof(double));
  B = (double *)aligned_alloc(64, NCOLS * NROWS * sizeof(double));
#else
  A = malloc(NROWS * NCOLS * sizeof(double));
  B = malloc(NCOLS * NROWS * sizeof(double));
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
  C = malloc(20000 * 20000 *sizeof(double));

  initArray(C, 20000, 20000);
#endif

  // transpose
#if defined INTRINSICS
  intrin8x8Transpose(A, B);
#elif defined BROWS && defined BCOLS
  blockedTranspose(A, B);
#else
  transpose(A, B);
#endif

#if defined PRINT_ARRAYS
  printf("B array:\n");
  printArray(B, NCOLS, NROWS);
#endif

  free(A);
  free(B);
#if defined CLEAR_L3_CACHE
  free(C);
#endif
}
