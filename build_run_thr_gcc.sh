#!/bin/bash

CC=gcc
CFLAGS="-O3 -Wall -std=c11"
THRFLAGS="-pthread"
AVXFLAGS="-vec-threshold0 -xCORE-AVX512 -qopt-zmm-usage=high"

CPPFLAGS=""
CPPFLAGS+=" -DCHECK_ARRAY"
#CPPFLAGS+=" -DCLEAR_L3_CACHE"
#CPPFLAGS+=" -DPRINT_ARRAYS"
echo ${CPPFLAGS}

DTYPE="double"
NTHREADS=2

for NROWS in 4096
do
    for NCOLS in 4096
    do
	for OPT in "ROW_NAIVE" "COL_NAIVE"
	do
	    PARAM_STR=${DTYPE}_${NROWS}x${NCOLS}_${NTHREADS}t_${OPT}
	    echo ${PARAM_STR}

            # compile and link
	    ${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o util.o util.c
	    ${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -D${OPT} -DNTHREADS=${NTHREADS} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o transpose_thr.o transpose_thr.c
	    ${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -D${OPT} -DNTHREADS=${NTHREADS} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o main_thr.o main_thr.c
	    ${CC} ${CFLAGS} ${CPPFLAGS} ${THRFLAGS} -o run_${PARAM_STR} util.o transpose_thr.o main_thr.o
	    
	    # run in parallel with one thread per core
	    numactl -C 0-"$((${NTHREADS}-1))" -l ./run_${PARAM_STR} > ./results_${PARAM_STR}.txt

	    # clean up
	    rm -rf run_${PARAM_STR} util.o transpose_thr.o main_thr.o *~
	done
    done
done
