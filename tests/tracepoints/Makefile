include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = tracepoint_alias

all: $(PROGRAM_LIST)

tracepoint_alias: tracepoint_alias.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/tracepoints
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/tracepoints

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
