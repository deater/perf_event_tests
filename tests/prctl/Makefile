include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	prctl \
	prctl_attach \
	prctl_child \
	prctl_inherit \
	prctl_parent




all: $(PROGRAM_LIST)

prctl: prctl.o $(LIB)/libhelper.a

prctl_attach: prctl_attach.o $(LIB)/libhelper.a

prctl_child: prctl_child.o $(LIB)/libhelper.a

prctl_inherit: prctl_inherit.o $(LIB)/libhelper.a

prctl_parent: prctl_parent.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/prctl
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/prctl

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
