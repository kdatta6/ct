#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "util.h"
#include "transpose_thr.h"

int main(int argc, char **argv) {
  DTYPE *A, *B;
#if defined CLEAR_L3_CACHE
  DTYPE *C;
#endif
  pthread_t threads[NTHREADS];
  threadArg *threadArgs;
  pthread_attr_t attr;
  struct timeval t1, t2;
  double elapsedTime;
  long t;
  void *status;
#if defined CHECK_ARRAY
  int rc;
#endif

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

  // start timer
  gettimeofday(&t1, NULL);

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
