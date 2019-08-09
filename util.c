#include <stdio.h>
#include <stdlib.h>

#include "util.h"

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
