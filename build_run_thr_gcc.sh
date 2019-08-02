#!/bin/bash

CC=gcc
CFLAGS="-O3 -Wall -std=c11"
THRFLAGS="-pthread"
AVXFLAGS="-vec-threshold0 -xCORE-AVX512 -qopt-zmm-usage=high"

# only cold cache numbers are collected because L3 cache is too small to get good warm cache results

NTHREADS=2

for OPT in ROW_NAIVE COL_NAIVE
do
    for NROWS in 8192
    do
        for NCOLS in 8192
        do
            # compile and run naive code with a cold cache (cc)
	    ${CC} ${CFLAGS} -DCHECK_ARRAY -D${OPT} -DNTHREADS=${NTHREADS} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o main_thr.o main_thr.c
	    ${CC} ${CFLAGS} -D${OPT} -DNTHREADS=${NTHREADS} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o transpose_thr.o transpose_thr.c
	    ${CC} ${CFLAGS} ${THRFLAGS} -o run_${NROWS}x${NCOLS}_${NTHREADS}t_${OPT} main_thr.o transpose_thr.o
	    
	    # run with one thread per core
	    numactl -C 0-"$((${NTHREADS}-1))" -l ./run_${NROWS}x${NCOLS}_${NTHREADS}t_${OPT} > ./results_${NROWS}x${NCOLS}_${NTHREADS}t_${OPT}.txt

	    # clean up
	    rm -rf run_${NROWS}x${NCOLS}_${NTHREADS}t_${OPT} main_thr.o transpose_thr.o *~
	done
    done
done
