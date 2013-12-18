CC = gcc
CFLAGS = -Wall -O2 -g -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	enable_on_exec \
	format_id_support \
	inherit \
	inherit_stat 


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

inherit:	inherit.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o 
	$(CC) $(LFLAGS) -o inherit inherit.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o -lpthread

inherit.o:	inherit.c
	$(CC) $(CFLAGS) -c inherit.c

###

inherit_stat:	inherit_stat.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o inherit_stat inherit_stat.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o \
				$(LIB)/perf_helpers.o -lpthread

inherit_stat.o:	inherit_stat.c
	$(CC) $(CFLAGS) -c inherit_stat.c


###

enable_on_exec:	enable_on_exec.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o enable_on_exec enable_on_exec.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

enable_on_exec.o:	enable_on_exec.c
	$(CC) $(CFLAGS) -c enable_on_exec.c


###                                                                            

format_id_support:	format_id_support.o \
	$(LIB)/perf_helpers.o \
	$(LIB)/test_utils.o \
	$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o format_id_support format_id_support.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o

format_id_support.o:	format_id_support.c
	$(CC) $(CFLAGS) -c format_id_support.c

###


clean:	
	rm -f *~ *.o \
	enable_on_exec \
	format_id_support \
	inherit \
	inherit_stat 

