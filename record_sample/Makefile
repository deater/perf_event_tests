CC = gcc
CFLAGS = -Wall -O2 -I../include
LFLAGS =

all:	\
	sample 

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

sample:	sample.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o sample sample.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/instructions_testcode.o

sample.o:	sample.c
	$(CC) $(CFLAGS) -c sample.c


clean:	
	rm -f *~ *.o \
	sample 

