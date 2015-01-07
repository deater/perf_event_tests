CC = gcc
CFLAGS = -Wall -O2 -I../../include
LFLAGS =
LIB = ../../lib

all:	socket_pass

socket_pass:	socket_pass.o
	$(CC) $(LFLAGS) -o socket_pass socket_pass.o \
			$(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o

socket_pass.o:	socket_pass.c
	$(CC) $(CFLAGS) -c socket_pass.c

clean:	
	rm -f *~ *.o socket_pass

