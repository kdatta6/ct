#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "transpose_thr.h"

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
  pthread_t threads[NTHREADS];
  threadArg *threadArgs;
  pthread_attr_t attr;
  long t;
  int rc;
  void *status;

  // initialize and set thread detached attribute
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  // allocate arrays
  A = malloc(NROWS * NCOLS * sizeof(DTYPE));
  B = malloc(NCOLS * NROWS * sizeof(DTYPE));

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

  threadArgs = malloc(NTHREADS * sizeof(threadArg));

  for (t=0; t<NTHREADS; t++) {
    (threadArgs+t)->A = A;
    (threadArgs+t)->B = B;
    (threadArgs+t)->t = t;

#if defined ROW_NAIVE
    rc = pthread_create(&threads[t], &attr, &rowThreadedTranspose_naive, (void *)(threadArgs+t));
#elif defined COL_NAIVE
    rc = pthread_create(&threads[t], &attr, &colThreadedTranspose_naive, (void *)(threadArgs+t));
#endif
    if (rc) {
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }
  // free attribute and wait for the other threads
  pthread_attr_destroy(&attr);
  for(t=0; t<NTHREADS; t++) {
    rc = pthread_join(threads[t], &status);
    if (rc) {
      printf("ERROR; return code from pthread_join() is %d\n", rc);
      exit(-1);
    }
  }

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
