CC = gcc
CFLAGS = -Wall -O2 -g -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	get_cache_info \


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

get_cache_info:	get_cache_info.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/detect_cache.o
	$(CC) $(LFLAGS) -o get_cache_info get_cache_info.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/detect_cache.o

get_cache_info.o:	get_cache_info.c
	$(CC) $(CFLAGS) -c get_cache_info.c

###


clean:	
	rm -f *~ *.o \
	get_cache_info 


