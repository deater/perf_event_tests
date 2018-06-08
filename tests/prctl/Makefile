include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	prctl



all: $(PROGRAM_LIST)

prctl: prctl.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/prctl
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/prctl

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
