CC = gcc
CFLAGS = -Wall -O2 -g -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	e2big \
	eacces \
	ebadf \
	efault \
	einval \
	emfile \
	enoent \
	esrch \
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

eacces:	eacces.o $(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o eacces eacces.o \
			$(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o

eacces.o:	eacces.c
	$(CC) $(CFLAGS) -c eacces.c

###

ebadf:	ebadf.o $(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o ebadf ebadf.o \
			$(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o

ebadf.o:	ebadf.c
	$(CC) $(CFLAGS) -c ebadf.c

###

efault:	efault.o $(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o efault efault.o \
			$(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o

efault.o:	efault.c
	$(CC) $(CFLAGS) -c efault.c

###

einval:	einval.o $(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o einval einval.o \
			$(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o

einval.o:	einval.c
	$(CC) $(CFLAGS) -c einval.c

###

emfile:	emfile.o $(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o emfile emfile.o \
			$(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o

emfile.o:	emfile.c
	$(CC) $(CFLAGS) -c emfile.c

###

enoent:	enoent.o $(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o enoent enoent.o \
			$(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o

enoent.o:	enoent.c
	$(CC) $(CFLAGS) -c enoent.c

###

esrch:	esrch.o $(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o esrch esrch.o \
			$(LIB)/test_utils.o \
			$(LIB)/perf_helpers.o

esrch.o:	esrch.c
	$(CC) $(CFLAGS) -c esrch.c

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
		eacces \
		ebadf \
		efault \
		einval \
		emfile \
		enoent \
		esrch \
		non-existent
