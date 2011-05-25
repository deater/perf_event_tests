CC = gcc
CFLAGS = -Wall -O2 -I../include
LFLAGS =

all:	branches instructions offcore_response \
	check_constraints simple_overflow

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

simple_overflow:	simple_overflow.o ../lib/test_utils.o \
					../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o simple_overflow simple_overflow.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o

simple_overflow.o:	simple_overflow.c
	$(CC) $(CFLAGS) -c simple_overflow.c

clean:	
	rm -f *~ *.o branches instructions offcore_response \
		check_constraints simple_overflow


