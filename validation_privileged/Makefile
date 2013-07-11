CC = gcc
CFLAGS = -Wall -O2 -I../include
LFLAGS =

all:	\
	uncore_events \
	uncore_cpu_cbox \
	uncore_max_cbox \
	uncore_group_cbox


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


###

uncore_max_cbox:	uncore_max_cbox.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o uncore_max_cbox uncore_max_cbox.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/matrix_multiply.o

uncore_max_cbox.o:	uncore_max_cbox.c
	$(CC) $(CFLAGS) -c uncore_max_cbox.c


###

uncore_group_cbox:	uncore_group_cbox.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o uncore_group_cbox uncore_group_cbox.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/matrix_multiply.o

uncore_group_cbox.o:	uncore_group_cbox.c
	$(CC) $(CFLAGS) -c uncore_group_cbox.c

###

uncore_cpu_cbox:	uncore_cpu_cbox.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o uncore_cpu_cbox uncore_cpu_cbox.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/matrix_multiply.o

uncore_cpu_cbox.o:	uncore_cpu_cbox.c
	$(CC) $(CFLAGS) -c uncore_cpu_cbox.c


####

clean:	
	rm -f *~ *.o \
	uncore_events \
	uncore_cpu_cbox \
	uncore_max_cbox \
	uncore_group_cbox
