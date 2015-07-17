include ../Makefile.globals.inc

CFLAGS += -Wall -O2 -g
LDFLAGS +=

PROGRAM_LIST = event_parser trace_event_parser trace_filter_generator

all: $(PROGRAM_LIST)

trace_filter_generator: trace_filter_generator.o
	$(CC) $(LDFLAGS) -o $@ $^

event_parser: event_parser.o
	$(CC) $(LDFLAGS) -o $@ $^

trace_event_parser: trace_event_parser.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o
	@- $(RM) *~
