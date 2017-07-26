include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	rdpmc_attach \
	rdpmc_attach_multi_enable \
	rdpmc_exec \
	rdpmc_exec_papi \
	rdpmc_group \
	rdpmc_multiattach \
	rdpmc_multiattach_papi \
	rdpmc_multiplexing \
	rdpmc_pthreads \
	rdpmc_pthreads_group \
	rdpmc_reset \
	rdpmc_support \
	rdpmc_sw_events \
	rdpmc_validation \
	rdpmc_comparision_readsyscall \
	rdpmc_comparision_mmap

all: $(PROGRAM_LIST)

rdpmc_attach: rdpmc_attach.o rdpmc_lib.o $(LIB)/libhelper.a

rdpmc_exec: rdpmc_exec.o rdpmc_lib.o $(LIB)/libhelper.a

rdpmc_exec_papi: rdpmc_exec_papi.o rdpmc_lib.o $(LIB)/libhelper.a

rdpmc_multiattach: rdpmc_multiattach.o rdpmc_lib.o $(LIB)/libhelper.a

rdpmc_multiattach_papi: rdpmc_multiattach_papi.o rdpmc_lib.o $(LIB)/libhelper.a

rdpmc_attach_multi_enable: rdpmc_attach_multi_enable.o rdpmc_lib.o $(LIB)/libhelper.a

rdpmc_group: rdpmc_group.o rdpmc_lib.o $(LIB)/libhelper.a

rdpmc_multiplexing: rdpmc_multiplexing.o rdpmc_lib.o $(LIB)/libhelper.a

rdpmc_pthreads: rdpmc_pthreads.o rdpmc_lib.o $(LIB)/libhelper.a -lpthread

rdpmc_pthreads_group: rdpmc_pthreads_group.o rdpmc_lib.o $(LIB)/libhelper.a -lpthread

rdpmc_reset: rdpmc_reset.o rdpmc_lib.o $(LIB)/libhelper.a

rdpmc_support: rdpmc_support.o rdpmc_lib.o $(LIB)/libhelper.a

rdpmc_sw_events: rdpmc_sw_events.o rdpmc_lib.o $(LIB)/libhelper.a

rdpmc_validation: rdpmc_validation.o rdpmc_lib.o $(LIB)/libhelper.a

rdpmc_comparision_readsyscall: rdpmc_comparision_readsyscall.o

rdpmc_comparision_mmap: rdpmc_comparision_mmap.o

install: all
	$(INSTALL) -d $(prefix)/tests/rdpmc
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/rdpmc

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
