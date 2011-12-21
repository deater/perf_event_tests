CC = gcc
CFLAGS = -Wall -O2 -I../include
LFLAGS =

all:	\
	offcore_response \
	get_cache_info \
	check_constraints \
	format_id_support \
	non-existent \
	rdpmc_support \
	rdpmc_validation \
	simple_overflow_leader \
	simple_overflow_sibling

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

offcore_response:	offcore_response.o ../lib/test_utils.o \
			../lib/matrix_multiply.o \
			../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o offcore_response offcore_response.o \
			../lib/test_utils.o \
			../lib/matrix_multiply.o \
			../lib/perf_helpers.o

offcore_response.o:	offcore_response.c
	$(CC) $(CFLAGS) -c offcore_response.c


###

check_constraints:	check_constraints.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o check_constraints check_constraints.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/matrix_multiply.o

check_constraints.o:	check_constraints.c
	$(CC) $(CFLAGS) -c check_constraints.c

###

get_cache_info:	get_cache_info.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/detect_cache.o
	$(CC) $(LFLAGS) -o get_cache_info get_cache_info.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/detect_cache.o

get_cache_info.o:	get_cache_info.c
	$(CC) $(CFLAGS) -c get_cache_info.c

###

non-existent:	non-existent.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o non-existent non-existent.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/matrix_multiply.o

non-existent.o:	non-existent.c
	$(CC) $(CFLAGS) -c non-existent.c


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

rdpmc_support:	rdpmc_support.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o rdpmc_support rdpmc_support.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/matrix_multiply.o

rdpmc_support.o:	rdpmc_support.c
	$(CC) $(CFLAGS) -c rdpmc_support.c

###

rdpmc_validation:	rdpmc_validation.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o rdpmc_validation rdpmc_validation.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/instructions_testcode.o

rdpmc_validation.o:	rdpmc_validation.c
	$(CC) $(CFLAGS) -c rdpmc_validation.c

###

simple_overflow_leader:	simple_overflow_leader.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o simple_overflow_leader simple_overflow_leader.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/matrix_multiply.o

simple_overflow_leader.o:	simple_overflow_leader.c
	$(CC) $(CFLAGS) -c simple_overflow_leader.c

###

simple_overflow_sibling:	simple_overflow_sibling.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o simple_overflow_sibling simple_overflow_sibling.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/matrix_multiply.o

simple_overflow_sibling.o:	simple_overflow_sibling.c
	$(CC) $(CFLAGS) -c simple_overflow_sibling.c


clean:	
	rm -f *~ *.o \
	offcore_response \
	get_cache_info \
	check_constraints \
	format_id_support \
	non-existent \
	rdpmc_support \
	rdpmc_validation \
	simple_overflow_leader \
	simple_overflow_sibling




