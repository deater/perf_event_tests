CC = gcc
CFLAGS = -Wall -O2
LFLAGS =

all:	simple_overflow

###

simple_overflow:	simple_overflow.o
	$(CC) $(LFLAGS) -o simple_overflow simple_overflow.o

simple_overflow.o:	simple_overflow.c
	$(CC) $(CFLAGS) -c simple_overflow.c

###

clean:	
	rm -f *~ *.o simple_overflow


