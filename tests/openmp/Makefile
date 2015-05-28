CC = gcc
CFLAGS = -Wall -O2 -I../../include -fopenmp
LFLAGS = -fopenmp
LIB = ../../lib

all:	openmp_test

openmp_test:	openmp_test.o \
		$(LIB)/test_utils.o \
		$(LIB)/instructions_testcode.o \
		$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o openmp_test openmp_test.o \
		$(LIB)/test_utils.o \
		$(LIB)/instructions_testcode.o \
		$(LIB)/perf_helpers.o

openmp_test.o:	openmp_test.c
	$(CC) $(CFLAGS) -c openmp_test.c

clean:	
	rm -f *~ *.o openmp_test

