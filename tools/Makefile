CC = gcc
CFLAGS = -Wall -O2 -g
LFLAGS =

all:	event_parser trace_event_parser trace_filter_generator

####

trace_filter_generator:	trace_filter_generator.o
	$(CC) $(LFLAGS) -o trace_filter_generator trace_filter_generator.o

trace_filter_generator.o:	trace_filter_generator.c trace_filters.h
	$(CC) $(CFLAGS) -c trace_filter_generator.c

####

event_parser:	event_parser.o
	$(CC) $(LFLAGS) -o event_parser event_parser.o

event_parser.o:	event_parser.c
	$(CC) $(CFLAGS) -c event_parser.c

####

trace_event_parser:	trace_event_parser.o
	$(CC) $(LFLAGS) -o trace_event_parser trace_event_parser.o

trace_event_parser.o:	trace_event_parser.c
	$(CC) $(CFLAGS) -c trace_event_parser.c

####



clean:	
	rm -f *~ *.o event_parser trace_event_parser trace_filter_generator

