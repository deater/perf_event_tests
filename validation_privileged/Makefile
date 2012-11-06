CC = gcc
CFLAGS = -Wall -O2 -I../include
LFLAGS =

all:	\
	uncore_events


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

uncore_events:	uncore_events.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o uncore_events uncore_events.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/matrix_multiply.o

uncore_events.o:	uncore_events.c
	$(CC) $(CFLAGS) -c uncore_events.c


clean:	
	rm -f *~ *.o \
	uncore_events


