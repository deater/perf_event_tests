CC = gcc
CFLAGS = -Wall -O2 -I../include
LFLAGS =

all:	branches branch-misses \
	l1-dcache-loads l1-dcache-stores instructions \
	offcore_response \
	check_constraints format_id_support \
	simple_overflow_leader simple_overflow_sibling

###

../lib/test_utils.o:	../lib/test_utils.c
	cd ../lib && make

../lib/branches_testcode.o:	../lib/branches_testcode.c
	cd ../lib && make

../lib/matrix_multiply.o:	../lib/matrix_multiply.c
	cd ../lib && make

../lib/perf_helpers.o:	../lib/perf_helpers.c
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
	rm -f *~ *.o branches branch-misses \
		l1-dcache-loads l1-dcache-stores instructions \
		offcore_response \
		check_constraints format_id_support \
		simple_overflow_leader simple_overflow_sibling



