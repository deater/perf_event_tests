CC = gcc
CFLAGS = -Wall -O2 -I../include
LFLAGS =

all:	sampled_notleader_refresh overflow_requires_mmap \
	format_id_support read_group_at_once check_schedulability \
	check_multiplexing

#

../lib/perf_helpers.o:	../lib/perf_helpers.c
	cd ../lib && make

../lib/test_utils.o:	../lib/test_utils.c
	cd ../lib && make

../lib/matrix_multiply.o:	../lib/matrix_multiply.c
	cd ../lib && make

###

overflow_requires_mmap:	overflow_requires_mmap.o \
			../lib/perf_helpers.o \
			../lib/test_utils.o
	$(CC) $(LFLAGS) -o overflow_requires_mmap overflow_requires_mmap.o \
			../lib/perf_helpers.o \
			../lib/test_utils.o

overflow_requires_mmap.o:	overflow_requires_mmap.c
	$(CC) $(CFLAGS) -c overflow_requires_mmap.c

###

sampled_notleader_refresh:	sampled_notleader_refresh.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o
	$(CC) $(LFLAGS) -o sampled_notleader_refresh sampled_notleader_refresh.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o

sampled_notleader_refresh.o:	sampled_notleader_refresh.c
	$(CC) $(CFLAGS) -c sampled_notleader_refresh.c

###

read_group_at_once:	read_group_at_once.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o read_group_at_once read_group_at_once.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o

read_group_at_once.o:	read_group_at_once.c
	$(CC) $(CFLAGS) -c read_group_at_once.c
	
###

format_id_support:	format_id_support.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o format_id_support format_id_support.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o

format_id_support.o:	format_id_support.c
	$(CC) $(CFLAGS) -c format_id_support.c
	
###

check_schedulability:	check_schedulability.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o check_schedulability check_schedulability.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o

check_schedulability.o:	check_schedulability.c
	$(CC) $(CFLAGS) -c check_schedulability.c
	
###	

check_multiplexing:	check_multiplexing.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o check_multiplexing check_multiplexing.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o

check_multiplexing.o:	check_multiplexing.c
	$(CC) $(CFLAGS) -c check_multiplexing.c
	
###

clean:	
	rm -f *~ *.o sampled_notleader_refresh overflow_requires_mmap \
	read_group_at_once format_id_support check_schedulability \
	check_multiplexing


