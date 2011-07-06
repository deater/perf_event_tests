CC = gcc
CFLAGS = -Wall -O2 -I../include

all:	branches_testcode.o instructions_testcode.o nops_testcode.o \
	matrix_multiply.o \
	detect_cache.o \
	perf_helpers.o test_utils.o

branches_testcode.o:	branches_testcode.c
	$(CC) $(CFLAGS) -c branches_testcode.c

instructions_testcode.o:	instructions_testcode.c
	$(CC) $(CFLAGS) -c instructions_testcode.c

nops_testcode.o:	nops_testcode.c
	$(CC) $(CFLAGS) -c nops_testcode.c

matrix_multiply.o:	matrix_multiply.c
	$(CC) $(CFLAGS) -c matrix_multiply.c


detect_cache.o:	detect_cache.c
	$(CC) $(CFLAGS) -c detect_cache.c

perf_helpers.o:	perf_helpers.c
	$(CC) $(CFLAGS) -c perf_helpers.c

test_utils.o:	test_utils.c
	$(CC) $(CFLAGS) -c test_utils.c

clean:	
	rm -f *~ *.o
