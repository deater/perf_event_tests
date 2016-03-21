include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	branches \
	branch-misses \
	cycles \
	instructions \
	l1-dcache-loads \
	l1-dcache-stores 

all: $(PROGRAM_LIST)


branches:branches.o $(LIB)/libhelper.a

branch-misses: branch-misses.o $(LIB)/libhelper.a

cycles: cycles.o $(LIB)/libhelper.a
	$(CC) $(LDFLAGS) -o $@ $^ -lrt

l1-dcache-loads: l1-dcache-loads.o $(LIB)/libhelper.a

l1-dcache-stores: l1-dcache-stores.o $(LIB)/libhelper.a

instructions: instructions.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/generalized_events
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/generalized_events

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
