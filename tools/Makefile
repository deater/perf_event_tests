CC = gcc
CFLAGS = -Wall -O2 -g
LFLAGS =

all:	event_parser

event_parser:	event_parser.o
	$(CC) $(LFLAGS) -o event_parser event_parser.o

event_parser.o:	event_parser.c
	$(CC) $(CFLAGS) -c event_parser.c

clean:	
	rm -f *~ *.o event_parser

