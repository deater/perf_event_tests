CC = gcc

CFLAGS = -Wall -O2 -I../include

CFLAGS_X32 = -mx32 -Wall -O2 -I../include


include ../Makefile.papi.inc

all:	branches_testcode.o instructions_testcode.o nops_testcode.o \
	matrix_multiply.o \
	detect_cache.o \
	perf_helpers.o \
	test_utils.o

x32:	perf_helpers.x32.o instructions_testcode.x32.o

papi:	papi_helpers.o

branches_testcode.o:	branches_testcode.c
	$(CC) $(CFLAGS) -c branches_testcode.c

####

instructions_testcode.o:	instructions_testcode.c
	$(CC) $(CFLAGS) -c instructions_testcode.c

instructions_testcode.x32.o:	instructions_testcode.c
	$(CC) $(CFLAGS_X32) -o instructions_testcode.x32.o -c instructions_testcode.c

####

nops_testcode.o:	nops_testcode.c
	$(CC) $(CFLAGS) -c nops_testcode.c

matrix_multiply.o:	matrix_multiply.c
	$(CC) $(CFLAGS) -c matrix_multiply.c


detect_cache.o:	detect_cache.c
	$(CC) $(CFLAGS) -c detect_cache.c

papi_helpers.o:	papi_helpers.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_helpers.c

#####

perf_helpers.o:	perf_helpers.c
	$(CC) $(CFLAGS) -c perf_helpers.c

perf_helpers.x32.o:	perf_helpers.c
	$(CC) $(CFLAGS_X32) -o perf_helpers.x32.o -c perf_helpers.c


#####

test_utils.o:	test_utils.c
	$(CC) $(CFLAGS) -c test_utils.c

clean:	
	rm -f *~ *.o
