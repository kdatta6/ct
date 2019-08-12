#!/bin/bash

CC=/opt/gcc-9.1.0/bin/gcc
CFLAGS="-O3 -Wall -std=c11"
AVXFLAGS="-march=skylake-avx512 -mtune=skylake-avx512 -ffast-math"
#AVXFLAGS="-vec-threshold0 -xCORE-AVX512 -qopt-zmm-usage=high"  # this is for icc
THRFLAGS="-pthread"

CPPFLAGS=""
CPPFLAGS+=" -DCHECK_ARRAY"
CPPFLAGS+=" -DCLEAR_L3_CACHE"
#CPPFLAGS+=" -DPRINT_ARRAYS"

# DTYPE can be either "float" or "double"

# MODE can be either "SERIAL" or "THREADS"

# OPT for SERIAL can be either "NAIVE", "BLOCKED", "INTRINSICS_noSS", or "INTRINSICS_SS"
# Note: if "BLOCKED" is chosen, specify BROWS and BCOLS
# if "INTRINSICS" is chosen, only double data type is currently supported

# OPT for THREADS can be either "NAIVE_ROW", "NAIVE_COL", 'BLOCKED_ROW",
# "BLOCKED_COL", "INTRIN_noSS_ROW", "INTRIN_noSS_COL", "INTRIN_SS_ROW",
# or "INTRIN_SS_COL"

for DTYPE in "double"
do
    for NROWS in 4096 8192 16384 32768 65536
    do
	for NCOLS in 1024 2048 4096 8192 16384
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
			PARAM_STR=${DTYPE}_${NROWS}x${NCOLS}_${MODE}_${OPT}_${BDIM}
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

		for OPT in "INTRINSICS_noSS" "INTRINSICS_SS"
		do
		    PARAM_STR=${DTYPE}_${NROWS}x${NCOLS}_${MODE}_${OPT}
		    echo ${PARAM_STR}
		    
		    # compile and link
		    ${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -c -o util.o util.c
		    ${CC} ${CFLAGS} ${CPPFLAGS} ${AVXFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -D${OPT} -c -o transpose.o transpose.c
		    ${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -D${OPT} -c -o main.o main.c
		    ${CC} ${CFLAGS} -o run_${PARAM_STR} util.o transpose.o main.o
		    
		    # run serially on a single core
		    numactl -C 0 -l ./run_${PARAM_STR} > ./results_${PARAM_STR}.txt
		    
		    # clean up
		    rm -rf run_${PARAM_STR} util.o transpose.o main.o *~
		done
	    done
	    
	    for MODE in "THREADS"
	    do
		for NTHREADS in 1 2 4 8 16
		do
		    for OPT in "NAIVE_ROW" "NAIVE_COL"
		    do
			PARAM_STR=${DTYPE}_${NROWS}x${NCOLS}_${NTHREADS}t_${OPT}
			echo ${PARAM_STR}
			
			# compile and link
			${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o util.o util.c
			${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -D${OPT} -DNTHREADS=${NTHREADS} -c -o transpose.o transpose.c
			${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -D${OPT} -DNTHREADS=${NTHREADS} -c -o main.o main.c
			${CC} ${CFLAGS} ${THRFLAGS} -o run_${PARAM_STR} util.o transpose.o main.o
			
			# run in parallel with one thread per core
			numactl -C 0-"$((${NTHREADS}-1))" -l ./run_${PARAM_STR} > ./results_${PARAM_STR}.txt
			
			# clean up
			rm -rf run_${PARAM_STR} util.o transpose.o main.o *~
		    done

		    for OPT in "BLOCKED_ROW" "BLOCKED_COL"
		    do
			for BDIM in 8 16 32 64 128 256
			do
			    PARAM_STR=${DTYPE}_${NROWS}x${NCOLS}_${NTHREADS}t_${OPT}_${BDIM}
			    echo ${PARAM_STR}
			
			    # compile and link
			    ${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o util.o util.c
			    ${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -DBROWS=${BDIM} -DBCOLS=${BDIM} -D${MODE} -D${OPT} -DNTHREADS=${NTHREADS} -c -o transpose.o transpose.c
			    ${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -D${OPT} -DNTHREADS=${NTHREADS} -c -o main.o main.c
			    ${CC} ${CFLAGS} ${THRFLAGS} -o run_${PARAM_STR} util.o transpose.o main.o
			    
			    # run in parallel with one thread per core
			    numactl -C 0-"$((${NTHREADS}-1))" -l ./run_${PARAM_STR} > ./results_${PARAM_STR}.txt
			
			    # clean up
			    rm -rf run_${PARAM_STR} util.o transpose.o main.o *~
			done
		    done

		    for OPT in "INTRIN_noSS_ROW" "INTRIN_SS_ROW" "INTRIN_noSS_COL" "INTRIN_SS_COL"
		    do
			PARAM_STR=${DTYPE}_${NROWS}x${NCOLS}_${NTHREADS}t_${OPT}
			echo ${PARAM_STR}
			
			# compile and link
			${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o util.o util.c
			${CC} ${CFLAGS} ${CPPFLAGS} ${AVXFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -D${OPT} -DNTHREADS=${NTHREADS} -c -o transpose.o transpose.c
			${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -D${MODE} -D${OPT} -DNTHREADS=${NTHREADS} -c -o main.o main.c
			${CC} ${CFLAGS} ${THRFLAGS} -o run_${PARAM_STR} util.o transpose.o main.o
			
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
