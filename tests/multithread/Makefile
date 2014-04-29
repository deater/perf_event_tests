CC = gcc
CFLAGS = -Wall -O2 -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	fork_then_read \
	lots_of_forks \
	open_fork_close_kill



#

$(LIB)/perf_helpers.o:	$(LIB)/perf_helpers.c
	cd $(LIB) && make

$(LIB)/test_utils.o:	$(LIB)/test_utils.c
	cd $(LIB) && make

$(LIB)/matrix_multiply.o:	$(LIB)/matrix_multiply.c
	cd $(LIB) && make

$(LIB)/instructions_testcode.o:	$(LIB)/instructions_testcode.c
	cd $(LIB) && make

###

fork_then_read:	fork_then_read.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o fork_then_read fork_then_read.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o

fork_then_read.o:	fork_then_read.c
	$(CC) $(CFLAGS) -c fork_then_read.c

####

lots_of_forks:	lots_of_forks.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o lots_of_forks lots_of_forks.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o

lots_of_forks.o:	lots_of_forks.c
	$(CC) $(CFLAGS) -c lots_of_forks.c


####

open_fork_close_kill:	open_fork_close_kill.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o open_fork_close_kill open_fork_close_kill.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o

open_fork_close_kill.o:	open_fork_close_kill.c
	$(CC) $(CFLAGS) -c open_fork_close_kill.c

####

clean:	
	rm -f *~ *.o \
	fork_then_read \
	lots_of_forks \
	open_fork_close_kill




