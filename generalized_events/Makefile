CC = gcc
CFLAGS = -Wall -O2 -I../include
LFLAGS =

all:	branches \
	branch-misses \
	cycles \
	instructions \
	l1-dcache-loads \
	l1-dcache-stores 



###

../lib/test_utils.o:	../lib/test_utils.c
	cd ../lib && make

../lib/branches_testcode.o:	../lib/branches_testcode.c
	cd ../lib && make

../lib/matrix_multiply.o:	../lib/matrix_multiply.c
	cd ../lib && make

../lib/perf_helpers.o:	../lib/perf_helpers.c
	cd ../lib && make

../lib/detect_cache.o:	../lib/detect_cache.c
	cd ../lib && make

###

branches:	branches.o ../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/branches_testcode.o
	$(CC) $(LFLAGS) -o branches branches.o \
			../lib/test_utils.o ../lib/branches_testcode.o \
			../lib/perf_helpers.o

branches.o:	branches.c
	$(CC) $(CFLAGS) -c branches.c

###


branch-misses:	branch-misses.o ../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/branches_testcode.o
	$(CC) $(LFLAGS) -o branch-misses branch-misses.o \
			../lib/test_utils.o ../lib/branches_testcode.o \
			../lib/perf_helpers.o

branch-misses.o:	branch-misses.c
	$(CC) $(CFLAGS) -c branch-misses.c

###

cycles:	cycles.o ../lib/test_utils.o \
			../lib/matrix_multiply.o \
			../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o cycles cycles.o \
			../lib/test_utils.o \
			../lib/matrix_multiply.o \
			../lib/perf_helpers.o -lrt

cycles.o:	cycles.c
	$(CC) $(CFLAGS) -c cycles.c

###

l1-dcache-loads:	l1-dcache-loads.o ../lib/test_utils.o \
				../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o l1-dcache-loads l1-dcache-loads.o \
			../lib/test_utils.o \
			../lib/perf_helpers.o

l1-dcache-loads.o:	l1-dcache-loads.c
	$(CC) $(CFLAGS) -c l1-dcache-loads.c

###

l1-dcache-stores:	l1-dcache-stores.o ../lib/test_utils.o \
				../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o l1-dcache-stores l1-dcache-stores.o \
			../lib/test_utils.o \
			../lib/perf_helpers.o

l1-dcache-stores.o:	l1-dcache-stores.c
	$(CC) $(CFLAGS) -c l1-dcache-stores.c

###

instructions:	instructions.o ../lib/test_utils.o \
			../lib/instructions_testcode.o \
			../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o instructions instructions.o \
			../lib/test_utils.o \
			../lib/instructions_testcode.o \
			../lib/perf_helpers.o

instructions.o:	instructions.c
	$(CC) $(CFLAGS) -c instructions.c


clean:	
	rm -f *~ *.o \
	branches \
	branch-misses \
	cycles \
	instructions \
	l1-dcache-loads \
	l1-dcache-stores 


