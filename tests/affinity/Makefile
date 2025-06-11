include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	affinity

all: $(PROGRAM_LIST)

affinity: affinity.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/ioctl
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/ioctl

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
