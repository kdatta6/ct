#!/bin/bash

CC=gcc
CFLAGS="-O3 -Wall -std=c11"
THRFLAGS="-pthread"
AVXFLAGS="-vec-threshold0 -xCORE-AVX512 -qopt-zmm-usage=high"

CPPFLAGS=""
CPPFLAGS+=" -DCHECK_ARRAY"
#CPPFLAGS+=" -DCLEAR_L3_CACHE"
#CPPFLAGS+=" -DPRINT_ARRAYS"

# DTYPE can be either "float" or "double"

# MODE can be either "SERIAL" or "THREADS"

# OPT for SERIAL can be either "NAIVE", "BLOCKED", or "INTRINSICS"
# Note: if "BLOCKED" is chosen, specify BROWS and BCOLS
# if "INTRINSICS" is chosen, only double data type is currently supported

# OPT for THREADS can be either "NAIVE_ROW" or "NAIVE_COL"

for DTYPE in "float" "double"
do
    for NROWS in 4096
    do
	for NCOLS in 4096
	do
	    for MODE in "SERIAL"
	    do
		for OPT in "NAIVE"
		do
		    PARAM_STR=${DTYPE}_${NROWS}x${NCOLS}_${MODE}_${OPT}
		    echo ${PARAM_STR}
		    
		    # compile and link
		    ${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -c -o util.o util.c
		    ${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -D${OPT} -c -o transpose.o transpose.c
		    ${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -D${OPT} -c -o main.o main.c
		    ${CC} ${CFLAGS} -o run_${PARAM_STR} util.o transpose.o main.o
		    
		    # run serially on a single core
		    numactl -C 0 -l ./run_${PARAM_STR} > ./results_${PARAM_STR}.txt
		    
		    # clean up
		    rm -rf run_${PARAM_STR} util.o transpose.o main.o *~
		done
		
		for OPT in "BLOCKED"
		do
		    for BDIM in 8 16 32 64 128 256
		    do
			PARAM_STR=${DTYPE}_${NROWS}x${NCOLS}_${BDIM}_${MODE}_${OPT}
			echo ${PARAM_STR}
			
			# compile and link
			${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -c -o util.o util.c
			${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -DBROWS=${BDIM} -DBCOLS=${BDIM} -D${MODE} -D${OPT} -c -o transpose.o transpose.c
			${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -D${OPT} -c -o main.o main.c
			${CC} ${CFLAGS} -o run_${PARAM_STR} util.o transpose.o main.o
			
			# run serially on a single core
			numactl -C 0 -l ./run_${PARAM_STR} > ./results_${PARAM_STR}.txt
			
			# clean up
			rm -rf run_${PARAM_STR} util.o transpose.o main.o *~
		    done
		done
	    done
	    
	    for MODE in "THREADS"
	    do
		for OPT in "NAIVE_ROW" "NAIVE_COL"
		do
		    for NTHREADS in 1 2
		    do
			PARAM_STR=${DTYPE}_${NROWS}x${NCOLS}_${NTHREADS}t_${OPT}
			echo ${PARAM_STR}
			
			# compile and link
			${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o util.o util.c
			${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -D${OPT} -DNTHREADS=${NTHREADS} -c -o transpose.o transpose.c
			${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -D${OPT} -DNTHREADS=${NTHREADS} -c -o main.o main.c
			${CC} ${CFLAGS} ${CPPFLAGS} ${THRFLAGS} -o run_${PARAM_STR} util.o transpose.o main.o
			
			# run in parallel with one thread per core
			numactl -C 0-"$((${NTHREADS}-1))" -l ./run_${PARAM_STR} > ./results_${PARAM_STR}.txt
			
			# clean up
			rm -rf run_${PARAM_STR} util.o transpose.o main.o *~
		    done
		done
	    done
	done
    done
done
