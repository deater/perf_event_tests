CC = gcc
CFLAGS = -Wall -O2 -I../../include
LFLAGS =
LIB = ../../lib

all:	events


###

$(LIB)/test_utils.o:	$(LIB)/test_utils.c
	cd $(LIB) && make

###

events:	events.o $(LIB)/test_utils.o
	$(CC) $(LFLAGS) -o events events.o $(LIB)/test_utils.o

events.o:	events.c
	$(CC) $(CFLAGS) -c events.c

###

clean:	
	rm -f *~ *.o \
		events

