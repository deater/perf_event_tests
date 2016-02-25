include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	rdpmc_support \
	rdpmc_validation \
	rdpmc_comparision_readsyscall

all: $(PROGRAM_LIST)


rdpmc_support: rdpmc_support.o rdpmc_lib.o $(LIB)/libhelper.a

rdpmc_validation: rdpmc_validation.o rdpmc_lib.o $(LIB)/libhelper.a

rdpmc_comparision_readsyscall: rdpmc_comparision_readsyscall.o

install: all
	$(INSTALL) -d $(prefix)/tests/rdpmc
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/rdpmc

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
