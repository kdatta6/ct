#!/bin/bash

CC=gcc
CFLAGS="-O3 -Wall -std=c11"
THRFLAGS="-pthread"
AVXFLAGS="-vec-threshold0 -xCORE-AVX512 -qopt-zmm-usage=high"

# only cold cache numbers are collected because L3 cache is too small to get good warm cache results

NTHREADS=3

for NROWS in 18
do
    for NCOLS in 8
    do
	# compile and run naive code with a cold cache (cc)
	${CC} ${CFLAGS} -DCLEAR_L3_CACHE -DPRINT_ARRAYS -DNTHREADS=${NTHREADS} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o main_thr.o main_thr.c
	${CC} ${CFLAGS} -DPRINT_ARRAYS -DNTHREADS=${NTHREADS} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o transpose_thr.o transpose_thr.c
	${CC} ${CFLAGS} ${THRFLAGS} -o run_${NROWS}x${NCOLS}_naive main_thr.o transpose_thr.o

	# run with one thread per core
	numactl -C 0-${NTHREADS} -l ./run_${NROWS}x${NCOLS}_naive > ./results_${NROWS}x${NCOLS}_naive.txt

	# clean up
	rm -rf run_${NROWS}x${NCOLS}_naive main_thr.o transpose_thr.o *~
    done
done
