include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = breakpoint_support

all: $(PROGRAM_LIST)

breakpoint_support: breakpoint_support.o $(LIB)/libhelper.a

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

install: all
	$(INSTALL) -d $(prefix)/tests/breakpoints
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/breakpoints

clean-local:
	@- $(RM) *~
