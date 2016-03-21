include ../Makefile.globals.inc

CFLAGS += -Wall -O2 -g
LDFLAGS +=

PROGRAM_LIST = event_parser trace_event_parser trace_filter_generator

all: $(PROGRAM_LIST)

trace_filter_generator: trace_filter_generator.o

event_parser: event_parser.o

trace_event_parser: trace_event_parser.o

install: all
	$(INSTALL) -d $(prefix)/tools
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tools

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
