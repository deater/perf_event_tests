CC = gcc
CFLAGS = -Wall -O2 -I../../include -fopenmp
LFLAGS = -fopenmp
LIB = ../../lib

all:	openmp_test openmp_overflow openmp_sample

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



openmp_overflow:	openmp_overflow.o \
		$(LIB)/test_utils.o \
		$(LIB)/instructions_testcode.o \
		$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o openmp_overflow openmp_overflow.o \
		$(LIB)/test_utils.o \
		$(LIB)/instructions_testcode.o \
		$(LIB)/perf_helpers.o

openmp_overflow.o:	openmp_overflow.c
	$(CC) $(CFLAGS) -c openmp_overflow.c

openmp_sample:	openmp_sample.o \
		$(LIB)/test_utils.o \
		$(LIB)/instructions_testcode.o \
		$(LIB)/perf_helpers.o \
		$(LIB)/parse_record.o
	$(CC) $(LFLAGS) -o openmp_sample openmp_sample.o \
		$(LIB)/test_utils.o \
		$(LIB)/instructions_testcode.o \
		$(LIB)/perf_helpers.o \
		$(LIB)/parse_record.o

openmp_sample.o:	openmp_sample.c
	$(CC) $(CFLAGS) -c openmp_sample.c


clean:	
	rm -f *~ *.o openmp_overflow openmp_test openmp_sample
