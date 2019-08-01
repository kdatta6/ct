CC = icc
CFLAGS += -O3 -Wall -std=c99
#CFLAGS += -O3 -Wall -std=c99 -qopt-streaming-stores=always
AVXFLAGS += -vec-threshold0 -xCORE-AVX512 -qopt-zmm-usage=high

BINS = transpose_ser transpose_ser_avx
OBJ = main_ser.o transpose_ser.o transpose_ser_avx.o
ASS = transpose_ser.S transpose_ser_avx.S

.PHONY: all
all: $(BINS) $(OBJ)

main_ser.o:	main_ser.c
		$(CC) $(CFLAGS) -c -o $@ $^

transpose_ser.o:	transpose_ser.c
		$(CC) $(CFLAGS) -qopt-report=5 -c -o $@ $^

transpose_ser:	main_ser.o transpose_ser.o
		$(CC) $(CFLAGS) -o $@ $^

transpose_ser.S:	transpose_ser.c
		$(CC) $(CFLAGS) -S -o $@ $^

transpose_ser_avx.o:	transpose_ser.c
			$(CC) $(CFLAGS) $(AVXFLAGS) -qopt-report=5 -c -o $@ $^

transpose_ser_avx:	main_ser.o transpose_ser_avx.o
		$(CC) $(CFLAGS) -o $@ $^

transpose_ser_avx.S:	transpose_ser.c
			$(CC) $(CFLAGS) $(AVXFLAGS) -S -o $@ $^

clean:
	rm -rf *~ *.optrpt *.txt run_* r*hs $(BINS) $(OBJ) $(ASS)
