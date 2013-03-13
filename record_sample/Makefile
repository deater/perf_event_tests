CC = gcc
CFLAGS = -Wall -O2 -I../include
LFLAGS =

all:	\
	lost_record_sample \
	multiple_mmap_sizes \
	print_record_sample \
	validate_record_sample

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

parse_record.o:		parse_record.c parse_record.h
	$(CC) $(CFLAGS) -c parse_record.c

###

lost_record_sample:	lost_record_sample.o \
					parse_record.o \
					../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o lost_record_sample \
				lost_record_sample.o \
				parse_record.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/instructions_testcode.o

lost_record_sample.o:	lost_record_sample.c
	$(CC) $(CFLAGS) -c lost_record_sample.c


###

multiple_mmap_sizes:	multiple_mmap_sizes.o \
					parse_record.o \
					../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o multiple_mmap_sizes \
				multiple_mmap_sizes.o \
				parse_record.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/instructions_testcode.o

multiple_mmap_sizes.o:	multiple_mmap_sizes.c
	$(CC) $(CFLAGS) -c multiple_mmap_sizes.c

###

print_record_sample:	print_record_sample.o \
					parse_record.o \
					../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o print_record_sample \
				print_record_sample.o \
				parse_record.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/instructions_testcode.o

print_record_sample.o:	print_record_sample.c
	$(CC) $(CFLAGS) -c print_record_sample.c

###

validate_record_sample:	validate_record_sample.o \
					parse_record.o \
					../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o validate_record_sample \
				validate_record_sample.o \
				parse_record.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/instructions_testcode.o

validate_record_sample.o:	validate_record_sample.c
	$(CC) $(CFLAGS) -c validate_record_sample.c


###

clean:	
	rm -f *~ *.o \
	lost_record_sample \
	multiple_mmap_sizes \
	print_record_sample \
	validate_record_sample

