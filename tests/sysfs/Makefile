include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = events

all: $(PROGRAM_LIST)

events: events.o $(LIB)/libhelper.a

clean:
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o
	@- $(RM) *~
