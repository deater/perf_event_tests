CC = gcc
CFLAGS = -Wall -O2 -g -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	check_multiplexing \
	check_papi_multiplexing \
	old_multiplexing


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

check_papi_multiplexing:	check_papi_multiplexing.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o check_papi_multiplexing check_papi_multiplexing.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o

check_papi_multiplexing.o:	check_papi_multiplexing.c
	$(CC) $(CFLAGS) -c check_papi_multiplexing.c

###	

check_multiplexing:	check_multiplexing.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o check_multiplexing check_multiplexing.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o

check_multiplexing.o:	check_multiplexing.c
	$(CC) $(CFLAGS) -c check_multiplexing.c



###

old_multiplexing:	old_multiplexing.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o old_multiplexing old_multiplexing.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/matrix_multiply.o

old_multiplexing.o:	old_multiplexing.c
	$(CC) $(CFLAGS) -c old_multiplexing.c

###


clean:	
	rm -f *~ *.o \
	check_multiplexing \
	check_papi_multiplexing \
	old_multiplexing
