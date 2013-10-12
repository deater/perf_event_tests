CC = gcc
CFLAGS = -Wall -O2 -g -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	rdpmc_support \
	rdpmc_validation 

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

rdpmc_lib.o:	rdpmc_lib.c rdpmc_lib.h
	$(CC) $(CFLAGS) -c rdpmc_lib.c

###

rdpmc_support:	rdpmc_support.o rdpmc_lib.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o rdpmc_support rdpmc_support.o \
				rdpmc_lib.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/matrix_multiply.o

rdpmc_support.o:	rdpmc_support.c
	$(CC) $(CFLAGS) -c rdpmc_support.c

###

rdpmc_validation:	rdpmc_validation.o rdpmc_lib.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o rdpmc_validation rdpmc_validation.o \
				rdpmc_lib.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

rdpmc_validation.o:	rdpmc_validation.c
	$(CC) $(CFLAGS) -c rdpmc_validation.c

###


clean:	
	rm -f *~ *.o \
	rdpmc_support \
	rdpmc_validation 
