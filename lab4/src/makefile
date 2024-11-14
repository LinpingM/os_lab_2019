CC=gcc
CFLAGS=-I.

all: programm

programm : sum_lib.o sum_lib.h
	$(CC) -o programm sum_lib.o parallel_sum.c -lpthread $(CFLAGS)

sum_lib : sum_lib.h
	$(CC) -o sum_lib.o -c sum_lib.c $(CFLAGS)

clean :
	rm sum_lib.o