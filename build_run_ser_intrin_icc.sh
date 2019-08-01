#!/bin/bash

source ~/.bash_profile

CFLAGS="-O3 -Wall -std=c11"
#CFLAGS="-O3 -Wall -std=c99"
AVXFLAGS="-vec-threshold0 -xCORE-AVX512 -qopt-zmm-usage=high"
SSFLAG="-qopt-streaming-stores=always"

# only cold cache numbers are collected because L3 cache is too small to get good warm cache results
# possibly do streaming stores?

#for NROWS in 8 16
for NROWS in 4096 8192 16384 32768 65536
do
#    for NCOLS in 8 16
    for NCOLS in 16384
    do
	# compile and run blocked code for cold cache (cc) only
	icc ${CFLAGS} -DINTRINSICS -DCLEAR_L3_CACHE -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o main1_cc.o main1.c
	icc ${CFLAGS} -DINTRINSICS -DNROWS=${NROWS} -DNCOLS=${NCOLS} -c -o transpose1.o transpose1.c
	icc ${CFLAGS} -o run_cc_${NROWS}x${NCOLS}_intrin_ss main1_cc.o transpose1.o
	amplxe-cl -collect hotspots numactl -C 0 -l ./run_cc_${NROWS}x${NCOLS}_intrin_ss > ./results_cc_${NROWS}x${NCOLS}_intrin_ss.txt
    done
done
