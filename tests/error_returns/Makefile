CC = gcc
CFLAGS = -Wall -O2 -g -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	e2big \
	efault \
	non-existent

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

e2big:	e2big.o $(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o e2big e2big.o \
			$(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o

e2big.o:	e2big.c
	$(CC) $(CFLAGS) -c e2big.c

###

efault:	efault.o $(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o efault efault.o \
			$(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o

efault.o:	efault.c
	$(CC) $(CFLAGS) -c efault.c

###

non-existent:	non-existent.o $(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o non-existent non-existent.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/matrix_multiply.o

non-existent.o:	non-existent.c
	$(CC) $(CFLAGS) -c non-existent.c

###


clean:	
	rm -f *~ *.o \
		e2big \
		efault \
		non-existent
