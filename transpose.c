#include <stdio.h>
#include <stdlib.h>
#if defined THREADS
#include <pthread.h>
#endif
#if ((defined INTRINSICS_noSS) || (defined INTRINSICS_SS) || (defined INTRIN_noSS_ROW) || (defined INTRIN_SS_ROW) || (defined INTRIN_noSS_COL) || (defined INTRIN_SS_COL))
#include <immintrin.h>
#include <stdint.h>
#endif

#include "transpose.h"

#if defined SERIAL
#if defined NAIVE
void naiveTranspose(DTYPE* restrict A, DTYPE* restrict B) {
  int i, j;
  for (i = 0; i < NROWS; i++) {
    for (j = 0; j < NCOLS; j++) {
      B[j*NROWS + i] = A[i*NCOLS + j];
    }
  }
}
#elif BLOCKED
/* This code only works if NROWS is a multiple of BROWS and
   NCOLS is a multiple of BCOLS. */
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
#elif ((defined INTRINSICS_noSS) || (defined INTRINSICS_SS))
/* This function uses intrinsics to transpose an 8x8 block of doubles
   using a recursive transpose algorithm.  It will not work correctly
   unless both NROWS and NCOLS are multiples of 8. */
void intrin8x8Transpose(double* restrict A, double* restrict B) {
  double *A_block, *B_block;
  int i_min, j_min;
  int num_row_blocks, num_col_blocks;
  int row_block_num, col_block_num;
  // alternate the reads and writes between the r and s vector registers, all of which hold matrix rows
  __m512d r0, r1, r2, r3, r4, r5, r6, r7;
  __m512d s0, s1, s2, s3, s4, s5, s6, s7;
  // the following are used to send the "idx" parameter to _mm512_permutex2var_pd
  uint64_t idx_arr[8];
  __m512i idx_2x2_0, idx_2x2_1, idx_4x4_0, idx_4x4_1;

  num_row_blocks = NROWS / 8;
  num_col_blocks = NCOLS / 8;

  // used for swapping 2x2 blocks using _mm512_permutex2var_pd()
  idx_arr[0] = 0x0000;
  idx_arr[1] = 0x0001;
  idx_arr[2] = 0x0008;
  idx_arr[3] = 0x0009;
  idx_arr[4] = 0x0004;
  idx_arr[5] = 0x0005;
  idx_arr[6] = 0x000c;
  idx_arr[7] = 0x000d;
  idx_2x2_0 = _mm512_loadu_si512(&idx_arr);

  idx_arr[0] = 0x000a;
  idx_arr[1] = 0x000b;
  idx_arr[2] = 0x0002;
  idx_arr[3] = 0x0003;
  idx_arr[4] = 0x000e;
  idx_arr[5] = 0x000f;
  idx_arr[6] = 0x0006;
  idx_arr[7] = 0x0007;
  idx_2x2_1 = _mm512_loadu_si512(&idx_arr);

  // used for swapping 4x4 blocks using _mm512_permutex2var_pd()
  idx_arr[0] = 0x0000;
  idx_arr[1] = 0x0001;
  idx_arr[2] = 0x0002;
  idx_arr[3] = 0x0003;
  idx_arr[4] = 0x0008;
  idx_arr[5] = 0x0009;
  idx_arr[6] = 0x000a;
  idx_arr[7] = 0x000b;
  idx_4x4_0 = _mm512_loadu_si512(&idx_arr);
  
  idx_arr[0] = 0x000c;
  idx_arr[1] = 0x000d;
  idx_arr[2] = 0x000e;
  idx_arr[3] = 0x000f;
  idx_arr[4] = 0x0004;
  idx_arr[5] = 0x0005;
  idx_arr[6] = 0x0006;
  idx_arr[7] = 0x0007;
  idx_4x4_1 = _mm512_loadu_si512(&idx_arr);

  // perform transpose over all blocks
  for (row_block_num = 0; row_block_num < num_row_blocks; row_block_num++) {
    i_min = row_block_num * 8;
    for (col_block_num = 0; col_block_num < num_col_blocks; col_block_num++) {
      j_min = col_block_num * 8;

      A_block = &A[i_min * NCOLS + j_min];
      B_block = &B[j_min * NROWS + i_min];

      // read 8x8 block of read array
      r0 = _mm512_load_pd(&A_block[0]);
      r1 = _mm512_load_pd(&A_block[NCOLS]);
      r2 = _mm512_load_pd(&A_block[2*NCOLS]);
      r3 = _mm512_load_pd(&A_block[3*NCOLS]);
      r4 = _mm512_load_pd(&A_block[4*NCOLS]);
      r5 = _mm512_load_pd(&A_block[5*NCOLS]);
      r6 = _mm512_load_pd(&A_block[6*NCOLS]);
      r7 = _mm512_load_pd(&A_block[7*NCOLS]);

      // shuffle doubles within 128-bit lanes
      s0 = _mm512_unpacklo_pd(r0, r1);
      s1 = _mm512_unpackhi_pd(r0, r1);
      s2 = _mm512_unpacklo_pd(r2, r3);
      s3 = _mm512_unpackhi_pd(r2, r3);
      s4 = _mm512_unpacklo_pd(r4, r5);
      s5 = _mm512_unpackhi_pd(r4, r5);
      s6 = _mm512_unpacklo_pd(r6, r7);
      s7 = _mm512_unpackhi_pd(r6, r7);

      // shuffle 2x2 blocks of doubles
      r0 = _mm512_permutex2var_pd(s0, idx_2x2_0, s2);
      r1 = _mm512_permutex2var_pd(s1, idx_2x2_0, s3);
      r2 = _mm512_permutex2var_pd(s2, idx_2x2_1, s0);
      r3 = _mm512_permutex2var_pd(s3, idx_2x2_1, s1);
      r4 = _mm512_permutex2var_pd(s4, idx_2x2_0, s6);
      r5 = _mm512_permutex2var_pd(s5, idx_2x2_0, s7);
      r6 = _mm512_permutex2var_pd(s6, idx_2x2_1, s4);
      r7 = _mm512_permutex2var_pd(s7, idx_2x2_1, s5);

      // shuffle 4x4 blocks of doubles
      s0 = _mm512_permutex2var_pd(r0, idx_4x4_0, r4);
      s1 = _mm512_permutex2var_pd(r1, idx_4x4_0, r5);
      s2 = _mm512_permutex2var_pd(r2, idx_4x4_0, r6);
      s3 = _mm512_permutex2var_pd(r3, idx_4x4_0, r7);
      s4 = _mm512_permutex2var_pd(r4, idx_4x4_1, r0);
      s5 = _mm512_permutex2var_pd(r5, idx_4x4_1, r1);
      s6 = _mm512_permutex2var_pd(r6, idx_4x4_1, r2);
      s7 = _mm512_permutex2var_pd(r7, idx_4x4_1, r3);

      // write back 8x8 block of write array
#if defined INTRINSICS_noSS
      _mm512_store_pd(&B_block[0], s0);
      _mm512_store_pd(&B_block[NROWS], s1);
      _mm512_store_pd(&B_block[2*NROWS], s2);
      _mm512_store_pd(&B_block[3*NROWS], s3);
      _mm512_store_pd(&B_block[4*NROWS], s4);
      _mm512_store_pd(&B_block[5*NROWS], s5);
      _mm512_store_pd(&B_block[6*NROWS], s6);
      _mm512_store_pd(&B_block[7*NROWS], s7);
#elif defined INTRINSICS_SS
      _mm512_stream_pd(&B_block[0], s0);
      _mm512_stream_pd(&B_block[NROWS], s1);
      _mm512_stream_pd(&B_block[2*NROWS], s2);
      _mm512_stream_pd(&B_block[3*NROWS], s3);
      _mm512_stream_pd(&B_block[4*NROWS], s4);
      _mm512_stream_pd(&B_block[5*NROWS], s5);
      _mm512_stream_pd(&B_block[6*NROWS], s6);
      _mm512_stream_pd(&B_block[7*NROWS], s7);
#endif
    }
  }
}
#endif
#elif THREADS
void *threadedTranspose(void *thrArg) {
  DTYPE * restrict A;
  DTYPE * restrict B;
  long tid;
  int num_large_chunks, small_chunk_size, large_chunk_size;
  int loop_min, loop_max;
#if (!(defined NAIVE_ROW) && !(defined NAIVE_COL))
  int row_block_num, col_block_num;
  int start_block_num, end_block_num;
  int i_min, j_min;
#endif
#if ((defined BLOCKED_ROW) || (defined BLOCKED_COL))
  int A_idx, B_idx;
#endif
#if ((defined BLOCKED_ROW) || (defined INTRIN_noSS_ROW) || (defined INTRIN_SS_ROW))
  int num_col_blocks;
#elif ((defined BLOCKED_COL) || (defined INTRIN_noSS_COL) || (defined INTRIN_SS_COL))
  int num_row_blocks;
#endif
#if ((defined INTRIN_noSS_ROW) || (defined INTRIN_SS_ROW) || (defined INTRIN_noSS_COL) || (defined INTRIN_SS_COL))
  double *A_block, *B_block;
  // alternate the reads and writes between the r and s vector registers, all of which hold matrix rows
  __m512d r0, r1, r2, r3, r4, r5, r6, r7;
  __m512d s0, s1, s2, s3, s4, s5, s6, s7;
  // the following are used to send the "idx" parameter to _mm512_permutex2var_pd
  uint64_t idx_arr[8];
  __m512i idx_2x2_0, idx_2x2_1, idx_4x4_0, idx_4x4_1;
#endif
#if ((defined NAIVE_ROW) || (defined NAIVE_COL) || (defined BLOCKED_ROW) || (defined BLOCKED_COL))
  int i, j;
#endif

  threadArg *my_thrArg = (threadArg *)thrArg;
  A = (DTYPE *)(my_thrArg->A);
  B = (DTYPE *)(my_thrArg->B);
  tid = (long)(my_thrArg->t);

  // divide the rows as evenly as possible among the threads
#if ((defined NAIVE_ROW) || (defined BLOCKED_ROW) || (defined INTRIN_noSS_ROW) || (defined INTRIN_SS_ROW))
  num_large_chunks = NROWS % NTHREADS;
  small_chunk_size = NROWS / NTHREADS;
#elif ((defined NAIVE_COL) || (defined BLOCKED_COL) || (defined INTRIN_noSS_COL) || (defined INTRIN_SS_COL))
  num_large_chunks = NCOLS % NTHREADS;
  small_chunk_size = NCOLS / NTHREADS;
#endif
  large_chunk_size = small_chunk_size + 1;
  
  if (tid < num_large_chunks) {
    loop_min = tid * large_chunk_size;
    loop_max = loop_min + large_chunk_size;
  }
  else {
    loop_min = num_large_chunks * large_chunk_size + (tid - num_large_chunks) * small_chunk_size;
    loop_max = loop_min + small_chunk_size;    
  }

#if defined PRINT_ARRAYS
#if ((defined NAIVE_ROW) || (defined BLOCKED_ROW))
  printf("In naiveRowThreadedTranspose(), tid = %ld, i_min = %d, i_max = %d\n", tid, i_min, i_max);
#elif ((defined NAIVE_COL) || (defined BLOCKED_COL))
  printf("In naiveColThreadedTranspose(), tid = %ld, j_min = %d, j_max = %d\n", tid, j_min, j_max);
#endif
#endif

#if defined NAIVE_ROW
  for (i = loop_min; i < loop_max; i++) {
    for (j = 0; j < NCOLS; j++) {
#elif defined NAIVE_COL
  for (i = 0; i < NROWS; i++) {
    for (j = loop_min; j < loop_max; j++) {
#endif
#if ((defined NAIVE_ROW) || (defined NAIVE_COL))
      B[j*NROWS + i] = A[i*NCOLS + j];
    }
  }
#endif

  // perform transpose over all blocks
#if defined BLOCKED_ROW
  num_col_blocks = NCOLS / BCOLS;
  start_block_num = loop_min / BROWS;
  end_block_num = loop_max / BROWS;
#elif ((defined INTRIN_noSS_ROW) || (defined INTRIN_SS_ROW))
  num_col_blocks = NCOLS / 8;
  start_block_num = loop_min / 8;
  end_block_num = loop_max / 8;
#endif

#if defined BLOCKED_COL
  num_row_blocks = NROWS / BROWS;
  start_block_num = loop_min / BCOLS;
  end_block_num = loop_max / BCOLS;
#elif ((defined INTRIN_noSS_COL) || (defined INTRIN_SS_COL))
  num_row_blocks = NROWS / 8;
  start_block_num = loop_min / 8;
  end_block_num = loop_max / 8;
#endif

#if ((defined INTRIN_noSS_ROW) || (defined INTRIN_SS_ROW) || (defined INTRIN_noSS_COL) || (defined INTRIN_SS_COL))
  // used for swapping 2x2 blocks using _mm512_permutex2var_pd()
  idx_arr[0] = 0x0000;
  idx_arr[1] = 0x0001;
  idx_arr[2] = 0x0008;
  idx_arr[3] = 0x0009;
  idx_arr[4] = 0x0004;
  idx_arr[5] = 0x0005;
  idx_arr[6] = 0x000c;
  idx_arr[7] = 0x000d;
  idx_2x2_0 = _mm512_loadu_si512(&idx_arr);

  idx_arr[0] = 0x000a;
  idx_arr[1] = 0x000b;
  idx_arr[2] = 0x0002;
  idx_arr[3] = 0x0003;
  idx_arr[4] = 0x000e;
  idx_arr[5] = 0x000f;
  idx_arr[6] = 0x0006;
  idx_arr[7] = 0x0007;
  idx_2x2_1 = _mm512_loadu_si512(&idx_arr);

  // used for swapping 4x4 blocks using _mm512_permutex2var_pd()
  idx_arr[0] = 0x0000;
  idx_arr[1] = 0x0001;
  idx_arr[2] = 0x0002;
  idx_arr[3] = 0x0003;
  idx_arr[4] = 0x0008;
  idx_arr[5] = 0x0009;
  idx_arr[6] = 0x000a;
  idx_arr[7] = 0x000b;
  idx_4x4_0 = _mm512_loadu_si512(&idx_arr);
  
  idx_arr[0] = 0x000c;
  idx_arr[1] = 0x000d;
  idx_arr[2] = 0x000e;
  idx_arr[3] = 0x000f;
  idx_arr[4] = 0x0004;
  idx_arr[5] = 0x0005;
  idx_arr[6] = 0x0006;
  idx_arr[7] = 0x0007;
  idx_4x4_1 = _mm512_loadu_si512(&idx_arr);
#endif

#if ((defined BLOCKED_ROW) || (defined INTRIN_noSS_ROW) || (defined INTRIN_SS_ROW))
  for (row_block_num = start_block_num; row_block_num < end_block_num; row_block_num++) {
    for (col_block_num = 0; col_block_num < num_col_blocks; col_block_num++) {
#elif ((defined BLOCKED_COL) || (defined INTRIN_noSS_COL) || (defined INTRIN_SS_COL))
  for (row_block_num = 0; row_block_num < num_row_blocks; row_block_num++) {
    for (col_block_num = start_block_num; col_block_num < end_block_num; col_block_num++) {
#endif
#if ((defined BLOCKED_ROW) || (defined BLOCKED_COL))
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
#elif ((defined INTRIN_noSS_ROW) || (defined INTRIN_SS_ROW) || (defined INTRIN_noSS_COL) || (defined INTRIN_SS_COL))
      i_min = row_block_num * 8;
      j_min = col_block_num * 8;

      A_block = &A[i_min * NCOLS + j_min];
      B_block = &B[j_min * NROWS + i_min];

      // read 8x8 block of read array
      r0 = _mm512_load_pd(&A_block[0]);
      r1 = _mm512_load_pd(&A_block[NCOLS]);
      r2 = _mm512_load_pd(&A_block[2*NCOLS]);
      r3 = _mm512_load_pd(&A_block[3*NCOLS]);
      r4 = _mm512_load_pd(&A_block[4*NCOLS]);
      r5 = _mm512_load_pd(&A_block[5*NCOLS]);
      r6 = _mm512_load_pd(&A_block[6*NCOLS]);
      r7 = _mm512_load_pd(&A_block[7*NCOLS]);

      // shuffle doubles within 128-bit lanes
      s0 = _mm512_unpacklo_pd(r0, r1);
      s1 = _mm512_unpackhi_pd(r0, r1);
      s2 = _mm512_unpacklo_pd(r2, r3);
      s3 = _mm512_unpackhi_pd(r2, r3);
      s4 = _mm512_unpacklo_pd(r4, r5);
      s5 = _mm512_unpackhi_pd(r4, r5);
      s6 = _mm512_unpacklo_pd(r6, r7);
      s7 = _mm512_unpackhi_pd(r6, r7);

      // shuffle 2x2 blocks of doubles
      r0 = _mm512_permutex2var_pd(s0, idx_2x2_0, s2);
      r1 = _mm512_permutex2var_pd(s1, idx_2x2_0, s3);
      r2 = _mm512_permutex2var_pd(s2, idx_2x2_1, s0);
      r3 = _mm512_permutex2var_pd(s3, idx_2x2_1, s1);
      r4 = _mm512_permutex2var_pd(s4, idx_2x2_0, s6);
      r5 = _mm512_permutex2var_pd(s5, idx_2x2_0, s7);
      r6 = _mm512_permutex2var_pd(s6, idx_2x2_1, s4);
      r7 = _mm512_permutex2var_pd(s7, idx_2x2_1, s5);

      // shuffle 4x4 blocks of doubles
      s0 = _mm512_permutex2var_pd(r0, idx_4x4_0, r4);
      s1 = _mm512_permutex2var_pd(r1, idx_4x4_0, r5);
      s2 = _mm512_permutex2var_pd(r2, idx_4x4_0, r6);
      s3 = _mm512_permutex2var_pd(r3, idx_4x4_0, r7);
      s4 = _mm512_permutex2var_pd(r4, idx_4x4_1, r0);
      s5 = _mm512_permutex2var_pd(r5, idx_4x4_1, r1);
      s6 = _mm512_permutex2var_pd(r6, idx_4x4_1, r2);
      s7 = _mm512_permutex2var_pd(r7, idx_4x4_1, r3);

      // write back 8x8 block of write array
#if ((defined INTRIN_noSS_ROW) || (defined INTRIN_noSS_COL))
      _mm512_store_pd(&B_block[0], s0);
      _mm512_store_pd(&B_block[NROWS], s1);
      _mm512_store_pd(&B_block[2*NROWS], s2);
      _mm512_store_pd(&B_block[3*NROWS], s3);
      _mm512_store_pd(&B_block[4*NROWS], s4);
      _mm512_store_pd(&B_block[5*NROWS], s5);
      _mm512_store_pd(&B_block[6*NROWS], s6);
      _mm512_store_pd(&B_block[7*NROWS], s7);
#elif ((defined INTRIN_SS_ROW) || (defined INTRIN_SS_COL))
      _mm512_stream_pd(&B_block[0], s0);
      _mm512_stream_pd(&B_block[NROWS], s1);
      _mm512_stream_pd(&B_block[2*NROWS], s2);
      _mm512_stream_pd(&B_block[3*NROWS], s3);
      _mm512_stream_pd(&B_block[4*NROWS], s4);
      _mm512_stream_pd(&B_block[5*NROWS], s5);
      _mm512_stream_pd(&B_block[6*NROWS], s6);
      _mm512_stream_pd(&B_block[7*NROWS], s7);
#endif
    }
  }
#endif
  
  pthread_exit((void*) tid);
}
#endif
