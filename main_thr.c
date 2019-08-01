#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "transpose_thr.h"

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
#if defined CLEAR_L3_CACHE
  double *C;
#endif
  pthread_t threads[NTHREADS];
  pthread_attr_t attr;
  long t;
  int rc;
  void *status;

  // initialize and set thread detached attribute
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  // allocate arrays
  A = malloc(NROWS * NCOLS * sizeof(double));
  B = malloc(NCOLS * NROWS * sizeof(double));

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

  // try rowThreadedTranspose_naive()
  for (t=0; t<NTHREADS; t++) {
    rc = pthread_create(&threads[t], &attr, &rowThreadedTranspose_naive, (void *)t);
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

  // try colThreadedTranspose_naive()  
  for (t=0; t<NTHREADS; t++) {
    rc = pthread_create(&threads[t], &attr, &colThreadedTranspose_naive, (void *)t);
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

  free(A);
  free(B);
#if defined CLEAR_L3_CACHE
  free(C);
#endif
}
