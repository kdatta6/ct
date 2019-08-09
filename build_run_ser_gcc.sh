#!/bin/bash

CC=gcc
CFLAGS="-O3 -Wall -std=c11"
AVXFLAGS="-vec-threshold0 -xCORE-AVX512 -qopt-zmm-usage=high"

# only cold cache numbers are collected because L3 cache is too small to get good warm cache results

DTYPE="double"

for OPT in "NAIVE"
do
    for NROWS in 4096
    do
	for NCOLS in 4096
	do
	    PARAM_STR=${DTYPE}_${NROWS}x${NCOLS}_${OPT}
	    echo ${PARAM_STR}
	    
	    # compile and run naive code with a cold cache (cc)
	    ${CC} ${CFLAGS} -DCHECK_ARRAY -DDTYPE=${DTYPE} -D${OPT} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o main_ser.o main_ser.c
	    ${CC} ${CFLAGS} -DDTYPE=${DTYPE} -D${OPT} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o transpose_ser.o transpose_ser.c
	    ${CC} ${CFLAGS} -o run_${PARAM_STR} main_ser.o transpose_ser.o

	    # run serially on a single core
	    numactl -C 0 -l ./run_${PARAM_STR} > ./results_${PARAM_STR}.txt

	    # clean up
	    rm -rf run_${PARAM_STR} main_ser.o transpose_ser.o *~
	done
    done
done
