#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "transpose_thr.h"

#if defined ROW_NAIVE
void *rowThreadedTranspose_naive(void *thrArg) {
  DTYPE * restrict A;
  DTYPE * restrict B;
  long tid;
  int num_large_chunks, small_chunk_size, large_chunk_size;
  int i_min, i_max;
  int i, j;

  threadArg *my_thrArg = (threadArg *)thrArg;
  A = (DTYPE *)(my_thrArg->A);
  B = (DTYPE *)(my_thrArg->B);
  tid = (long)(my_thrArg->t);

  // divide the rows as evenly as possible among the threads
  num_large_chunks = NROWS % NTHREADS;
  small_chunk_size = NROWS / NTHREADS;
  large_chunk_size = small_chunk_size + 1;
  
  if (tid < num_large_chunks) {
    i_min = tid * large_chunk_size;
    i_max = i_min + large_chunk_size;
  }
  else {
    i_min = num_large_chunks * large_chunk_size + (tid - num_large_chunks) * small_chunk_size;
    i_max = i_min + small_chunk_size;    
  }

#if defined PRINT_ARRAYS
  printf("In rowThreadedTranspose_naive(), tid = %ld, i_min = %d, i_max = %d\n", tid, i_min, i_max);
#endif
  for (i = i_min; i < i_max; i++) {
    for (j = 0; j < NCOLS; j++) {
      B[j*NROWS + i] = A[i*NCOLS + j];
    }
  }  
  
  pthread_exit((void*) tid);
}
#elif defined COL_NAIVE
void *colThreadedTranspose_naive(void *thrArg) {
  DTYPE * restrict A;
  DTYPE * restrict B;
  long tid;
  int num_large_chunks, small_chunk_size, large_chunk_size;
  int j_min, j_max;
  int i, j;

  threadArg *my_thrArg = (threadArg *)thrArg;
  A = (DTYPE *)(my_thrArg->A);
  B = (DTYPE *)(my_thrArg->B);
  tid = (long)(my_thrArg->t);

  // divide the rows as evenly as possible among the threads
  num_large_chunks = NCOLS % NTHREADS;
  small_chunk_size = NCOLS / NTHREADS;
  large_chunk_size = small_chunk_size + 1;
  
  if (tid < num_large_chunks) {
    j_min = tid * large_chunk_size;
    j_max = j_min + large_chunk_size;
  }
  else {
    j_min = num_large_chunks * large_chunk_size + (tid - num_large_chunks) * small_chunk_size;
    j_max = j_min + small_chunk_size;    
  }

#if defined PRINT_ARRAYS
  printf("In colThreadedTranspose_naive(), tid = %ld, j_min = %d, j_max = %d\n", tid, j_min, j_max);
#endif

  for (i = 0; i < NROWS; i++) {
    for (j = j_min; j < j_max; j++) {
      B[j*NROWS + i] = A[i*NCOLS + j];
    }
  }
  
  pthread_exit((void*) tid);
}
#endif

#if defined BROWS && defined BCOLS
// this code only works if NROWS is a multiple of BROWS and NCOLS is a multiple of BCOLS
void blockedTranspose(DTYPE* restrict A, DTYPE* restrict B) {
  int i, j, i_min, j_min;
  int num_row_blocks, num_col_blocks;
  int row_block_num, col_block_num;
  int A_idx, B_idx;

  num_row_blocks = NROWS / BROWS;
  num_col_blocks = NCOLS / BCOLS;

  // perform transpose over all blocks
  for (row_block_num = 0; row_block_num < num_row_blocks; row_block_num++) {
    for (col_block_num = 0; col_block_num < num_col_blocks; col_block_num++) {
      i_min = row_block_num * BROWS;
      j_min = col_block_num * BCOLS;

      for (i = i_min; i < (i_min + BROWS); i++) {
	A_idx = i * NCOLS + j_min;
	B_idx = j_min * NROWS + i;
	for (j = 0; j < BCOLS; j++) {
	  B[B_idx] = A[A_idx++];
	  B_idx += NROWS;
	}
      }
    }
  }
}
#endif
