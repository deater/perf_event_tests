CC = gcc
CFLAGS = -Wall -O2 -g -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	offcore_response \
	offcore_response_mask 


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

offcore_response:	offcore_response.o $(LIB)/test_utils.o \
			$(LIB)/matrix_multiply.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o offcore_response offcore_response.o \
			$(LIB)/test_utils.o \
			$(LIB)/matrix_multiply.o \
			$(LIB)/perf_helpers.o

offcore_response.o:	offcore_response.c
	$(CC) $(CFLAGS) -c offcore_response.c



###

offcore_response_mask:	offcore_response_mask.o $(LIB)/test_utils.o \
			$(LIB)/matrix_multiply.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o offcore_response_mask offcore_response_mask.o \
			$(LIB)/test_utils.o \
			$(LIB)/matrix_multiply.o \
			$(LIB)/perf_helpers.o

offcore_response_mask.o:	offcore_response_mask.c
	$(CC) $(CFLAGS) -c offcore_response_mask.c


###


clean:	
	rm -f *~ *.o \
	offcore_response \
	offcore_response_mask 

