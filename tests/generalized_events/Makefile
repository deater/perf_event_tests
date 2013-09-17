CC = gcc
CFLAGS = -Wall -O2 -I../../include
LFLAGS =
LIB = ../../lib

all:	branches \
	branch-misses \
	cycles \
	instructions \
	l1-dcache-loads \
	l1-dcache-stores 



###

$(LIB)/test_utils.o:	$(LIB)/test_utils.c
	cd $(LIB) && make

$(LIB)/branches_testcode.o:	$(LIB)/branches_testcode.c
	cd $(LIB) && make

$(LIB)/matrix_multiply.o:	$(LIB)/matrix_multiply.c
	cd $(LIB) && make

$(LIB)/perf_helpers.o:	$(LIB)/perf_helpers.c
	cd $(LIB) && make

$(LIB)/detect_cache.o:	$(LIB)/detect_cache.c
	cd $(LIB) && make

###

branches:	branches.o $(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/branches_testcode.o
	$(CC) $(LFLAGS) -o branches branches.o \
			$(LIB)/test_utils.o $(LIB)/branches_testcode.o \
			$(LIB)/perf_helpers.o

branches.o:	branches.c
	$(CC) $(CFLAGS) -c branches.c

###


branch-misses:	branch-misses.o $(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/branches_testcode.o
	$(CC) $(LFLAGS) -o branch-misses branch-misses.o \
			$(LIB)/test_utils.o $(LIB)/branches_testcode.o \
			$(LIB)/perf_helpers.o

branch-misses.o:	branch-misses.c
	$(CC) $(CFLAGS) -c branch-misses.c

###

cycles:	cycles.o $(LIB)/test_utils.o \
			$(LIB)/matrix_multiply.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o cycles cycles.o \
			$(LIB)/test_utils.o \
			$(LIB)/matrix_multiply.o \
			$(LIB)/perf_helpers.o -lrt

cycles.o:	cycles.c
	$(CC) $(CFLAGS) -c cycles.c

###

l1-dcache-loads:	l1-dcache-loads.o $(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o l1-dcache-loads l1-dcache-loads.o \
			$(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o

l1-dcache-loads.o:	l1-dcache-loads.c
	$(CC) $(CFLAGS) -c l1-dcache-loads.c

###

l1-dcache-stores:	l1-dcache-stores.o $(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o l1-dcache-stores l1-dcache-stores.o \
			$(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o

l1-dcache-stores.o:	l1-dcache-stores.c
	$(CC) $(CFLAGS) -c l1-dcache-stores.c

###

instructions:	instructions.o $(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o instructions instructions.o \
			$(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o

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


