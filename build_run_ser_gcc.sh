#!/bin/bash

CC=gcc
CFLAGS="-O3 -Wall -std=c11"
AVXFLAGS="-vec-threshold0 -xCORE-AVX512 -qopt-zmm-usage=high"

CPPFLAGS=""
CPPFLAGS+=" -DCHECK_ARRAY"
#CPPFLAGS+=" -DCLEAR_L3_CACHE"
#CPPFLAGS+=" -DPRINT_ARRAYS"
echo ${CPPFLAGS}

DTYPE="double"

# OPT include "NAIVE", "BLOCKED", and "INTRINSICS"
# Note: if "BLOCKED" is chosen, specify BROWS and BCOLS
# if "INTRINSICS" is chosen, only double data type is currently supported
for NROWS in 4096
do
    for NCOLS in 4096
    do
	# collect naive results
	for OPT in "NAIVE"
	do
	    PARAM_STR=${DTYPE}_${NROWS}x${NCOLS}_${OPT}
	    echo ${PARAM_STR}
	    
	    # compile and link
	    ${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o util.o util.c
	    ${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -D${OPT} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o transpose_ser.o transpose_ser.c
	    ${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -D${OPT} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o main_ser.o main_ser.c
	    ${CC} ${CFLAGS} -o run_${PARAM_STR} util.o transpose_ser.o main_ser.o

	    # run serially on a single core
	    numactl -C 0 -l ./run_${PARAM_STR} > ./results_${PARAM_STR}.txt

	    # clean up
	    rm -rf run_${PARAM_STR} util.o transpose_ser.o main_ser.o *~
	done

	# collect blocked results
	for OPT in "BLOCKED"
	do
	    for BDIM in 8 16 32 64 128 256
	    do
		PARAM_STR=${DTYPE}_${NROWS}x${NCOLS}_${BDIM}_${OPT}
		echo ${PARAM_STR}

		# compile and link
		${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o util.o util.c
		${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -D${OPT} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -DBROWS=${BDIM} -DBCOLS=${BDIM} -c -o transpose_ser.o transpose_ser.c
		${CC} ${CFLAGS} ${CPPFLAGS} -DDTYPE=${DTYPE} -D${OPT} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o main_ser.o main_ser.c
		${CC} ${CFLAGS} -o run_${PARAM_STR} util.o transpose_ser.o main_ser.o

		# run serially on a single core
		numactl -C 0 -l ./run_${PARAM_STR} > ./results_${PARAM_STR}.txt

		# clean up
		rm -rf run_${PARAM_STR} util.o transpose_ser.o main_ser.o *~
	    done
	done
    done
done
