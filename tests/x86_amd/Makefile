include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	ibs_fetch ibs_op

all: $(PROGRAM_LIST)

ibs_fetch: ibs_fetch.o $(LIB)/libhelper.a

ibs_op: ibs_op.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/x86_amd
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/x86_amd

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
