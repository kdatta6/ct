void initArray(DTYPE * restrict C, int nrows, int ncols);
void zeroArray(DTYPE * restrict C, int nrows, int ncols);
#if defined PRINT_ARRAYS
void printArray(DTYPE * restrict C, int nrows, int ncols);
#endif
#if defined CHECK_ARRAY
int checkArray(DTYPE * restrict C, DTYPE * restrict D, int nrows, int ncols);
#endif
