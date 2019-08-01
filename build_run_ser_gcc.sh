#!/bin/bash

#source ~/.bash_profile

CC=gcc
CFLAGS="-O3 -Wall -std=c11"
AVXFLAGS="-vec-threshold0 -xCORE-AVX512 -qopt-zmm-usage=high"

# only cold cache numbers are collected because L3 cache is too small to get good warm cache results

for NROWS in 16
do
    for NCOLS in 8
    do
	# compile and run naive code with a cold cache (cc)
	${CC} ${CFLAGS} -DCLEAR_L3_CACHE -DPRINT_ARRAYS -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o main_ser.o main_ser.c
	${CC} ${CFLAGS} -DPRINT_ARRAYS -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o transpose_ser.o transpose_ser.c
	${CC} ${CFLAGS} -o run_${NROWS}x${NCOLS}_naive main_ser.o transpose_ser.o
	numactl -C 0 -l ./run_${NROWS}x${NCOLS}_naive > ./results_${NROWS}x${NCOLS}_naive.txt
    done
done
