#!/bin/bash

source ~/.bash_profile

CFLAGS="-O3 -Wall -std=c11"
#CFLAGS="-O3 -Wall -std=c99"
AVXFLAGS="-vec-threshold0 -xCORE-AVX512 -qopt-zmm-usage=high"
#SSFLAG="-qopt-streaming-stores=always"

# only cold cache numbers are collected because L3 cache is too small to get good warm cache results

# change NCOLS as appropriate below

for NROWS in 8192 16384 32768 65536 131072
do
    for NCOLS in 8192
    do
	# compile and run naive code with a cold cache (cc)
	icc ${CFLAGS} -DCLEAR_L3_CACHE -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o main_ser_cc.o main_ser.c
	icc ${CFLAGS} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o transpose_ser.o transpose_ser.c
	icc ${CFLAGS} -o run_cc_${NROWS}x${NCOLS}_naive main_ser_cc.o transpose_ser.o
	amplxe-cl -collect hotspots numactl -C 0 -l ./run_cc_${NROWS}x${NCOLS}_naive > ./results_cc_${NROWS}x${NCOLS}_naive.txt
	
	icc ${CFLAGS} ${AVXFLAGS} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o transpose_ser_avx.o transpose_ser.c
	icc ${CFLAGS} -o run_cc_avx_${NROWS}x${NCOLS}_naive main_ser_cc.o transpose_ser_avx.o
	amplxe-cl -collect hotspots numactl -C 0 -l ./run_cc_avx_${NROWS}x${NCOLS}_naive > ./results_cc_avx_${NROWS}x${NCOLS}_naive.txt

	rm main_ser_cc.o transpose_ser.o transpose_ser_avx.o run_cc_${NROWS}x${NCOLS}_naive run_cc_avx_${NROWS}x${NCOLS}_naive

	# compile and run blocked code (with block size BxB) with a cold cache (cc)
	for B in 8 16 32 64 128 256 512
	do
	    icc ${CFLAGS} -DCLEAR_L3_CACHE -DNROWS=${NROWS} -DNCOLS=${NCOLS} -DBROWS=${B} -DBCOLS=${B} -c -o main_ser_cc.o main_ser.c
	    icc ${CFLAGS} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -DBROWS=${B} -DBCOLS=${B} -c -o transpose_ser.o transpose_ser.c
	    icc ${CFLAGS} -o run_cc_${NROWS}x${NCOLS}_${B} main_ser_cc.o transpose_ser.o
	    amplxe-cl -collect hotspots numactl -C 0 -l ./run_cc_${NROWS}x${NCOLS}_${B} > ./results_cc_${NROWS}x${NCOLS}_${B}.txt
	    
	    icc ${CFLAGS} ${AVXFLAGS} -DNROWS=${NROWS} -DNCOLS=${NCOLS} -DBROWS=${B} -DBCOLS=${B} -c -o transpose_ser_avx.o transpose_ser.c
	    icc ${CFLAGS} -o run_cc_avx_${NROWS}x${NCOLS}_${B} main_ser_cc.o transpose_ser_avx.o
	    amplxe-cl -collect hotspots numactl -C 0 -l ./run_cc_avx_${NROWS}x${NCOLS}_${B} > ./results_cc_avx_${NROWS}x${NCOLS}_${B}.txt

	    rm main_ser_cc.o transpose_ser.o transpose_ser_avx.o run_cc_${NROWS}x${NCOLS}_${B} run_cc_avx_${NROWS}x${NCOLS}_${B}
	done

	# compile and run intrinsics code with a cold cache (cc)
	icc ${CFLAGS} -DINTRINSICS -DCLEAR_L3_CACHE -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o main_ser_cc.o main_ser.c
	icc ${CFLAGS} -DINTRINSICS -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o transpose_ser.o transpose_ser.c
	icc ${CFLAGS} -o run_cc_${NROWS}x${NCOLS}_intrin_ss main_ser_cc.o transpose_ser.o
	amplxe-cl -collect hotspots numactl -C 0 -l ./run_cc_${NROWS}x${NCOLS}_intrin_ss > ./results_cc_${NROWS}x${NCOLS}_intrin_ss.txt
    done
done
