CC = gcc
CFLAGS = -Wall -O2 -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	tracepoint_alias


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

tracepoint_alias:	tracepoint_alias.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o 
	$(CC) $(LFLAGS) -o tracepoint_alias tracepoint_alias.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o 

tracepoint_alias.o:	tracepoint_alias.c
	$(CC) $(CFLAGS) -c tracepoint_alias.c


clean:	
	rm -f *~ *.o \
	tracepoint_alias 
