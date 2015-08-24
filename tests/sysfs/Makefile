include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = events

all: $(PROGRAM_LIST)

events: events.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/sysfs
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/sysfs

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
