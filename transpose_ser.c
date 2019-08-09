#include <stdio.h>
#include <stdlib.h>

#if defined INTRINSICS
#include <immintrin.h>
#include <stdint.h>
#endif

#include "transpose_ser.h"

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
#elif defined INTRINSICS
/* This function uses intrinsics to transpose an 8x8 block of doubles
   using a recursive transpose algorithm.  It will not work correctly
   unless both NROWS and NCOLS are multiples of 8. */
void intrin8x8Transpose(double* restrict A, double* restrict B) {
  double *A_block, *B_block;
  int i, j, i_min, j_min;
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
  idx_2x2_0 = _mm512_loadu_epi64(&idx_arr);

  idx_arr[0] = 0x000a;
  idx_arr[1] = 0x000b;
  idx_arr[2] = 0x0002;
  idx_arr[3] = 0x0003;
  idx_arr[4] = 0x000e;
  idx_arr[5] = 0x000f;
  idx_arr[6] = 0x0006;
  idx_arr[7] = 0x0007;
  idx_2x2_1 = _mm512_loadu_epi64(&idx_arr);

  // used for swapping 4x4 blocks using _mm512_permutex2var_pd()
  idx_arr[0] = 0x0000;
  idx_arr[1] = 0x0001;
  idx_arr[2] = 0x0002;
  idx_arr[3] = 0x0003;
  idx_arr[4] = 0x0008;
  idx_arr[5] = 0x0009;
  idx_arr[6] = 0x000a;
  idx_arr[7] = 0x000b;
  idx_4x4_0 = _mm512_loadu_epi64(&idx_arr);
  
  idx_arr[0] = 0x000c;
  idx_arr[1] = 0x000d;
  idx_arr[2] = 0x000e;
  idx_arr[3] = 0x000f;
  idx_arr[4] = 0x0004;
  idx_arr[5] = 0x0005;
  idx_arr[6] = 0x0006;
  idx_arr[7] = 0x0007;
  idx_4x4_1 = _mm512_loadu_epi64(&idx_arr);

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
      #ifdef STREAMING_STORES
      _mm512_stream_pd(&B_block[0], s0);
      _mm512_stream_pd(&B_block[NROWS], s1);
      _mm512_stream_pd(&B_block[2*NROWS], s2);
      _mm512_stream_pd(&B_block[3*NROWS], s3);
      _mm512_stream_pd(&B_block[4*NROWS], s4);
      _mm512_stream_pd(&B_block[5*NROWS], s5);
      _mm512_stream_pd(&B_block[6*NROWS], s6);
      _mm512_stream_pd(&B_block[7*NROWS], s7);
      #else
      _mm512_store_pd(&B_block[0], s0);
      _mm512_store_pd(&B_block[NROWS], s1);
      _mm512_store_pd(&B_block[2*NROWS], s2);
      _mm512_store_pd(&B_block[3*NROWS], s3);
      _mm512_store_pd(&B_block[4*NROWS], s4);
      _mm512_store_pd(&B_block[5*NROWS], s5);
      _mm512_store_pd(&B_block[6*NROWS], s6);
      _mm512_store_pd(&B_block[7*NROWS], s7);
      #endif
    }
  }
}
#endif
