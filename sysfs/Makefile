CC = gcc
CFLAGS = -Wall -O2 -I../include
LFLAGS =

all:	events


###

../lib/test_utils.o:	../lib/test_utils.c
	cd ../lib && make

###

events:	events.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o events events.o ../lib/test_utils.o

events.o:	events.c
	$(CC) $(CFLAGS) -c events.c

###

clean:	
	rm -f *~ *.o \
		events

