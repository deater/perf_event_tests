CC = gcc
CFLAGS = -Wall -O2 -g -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	breakpoint_support 

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

breakpoint_support:	breakpoint_support.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o 
	$(CC) $(LFLAGS) -o breakpoint_support breakpoint_support.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o 

breakpoint_support.o:	breakpoint_support.c
	$(CC) $(CFLAGS) -c breakpoint_support.c

###


clean:	
	rm -f *~ *.o \
	breakpoint_support 
