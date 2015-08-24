include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	bts_aux \
	bts_kernel \
	fixed_ctr0 \
	fixed_ctr1 \
	fixed_ctr2 \
	offcore_response \
	offcore_response_mask \
	pebs \
	pebs_addr \
	pebs_latency \
	pebs_registers \
	uncore_events \
	uncore_cpu_cbox \
	uncore_max_cbox \
	uncore_group_cbox

all: $(PROGRAM_LIST)

fixed_ctr0: fixed_ctr0.o $(LIB)/libhelper.a

fixed_ctr1: fixed_ctr1.o $(LIB)/libhelper.a

fixed_ctr2: fixed_ctr2.o $(LIB)/libhelper.a

offcore_response: offcore_response.o $(LIB)/libhelper.a

pebs: pebs.o $(LIB)/libhelper.a

pebs_addr: pebs_addr.o $(LIB)/libhelper.a

bts_aux: bts_aux.o $(LIB)/libhelper.a

bts_kernel: bts_kernel.o $(LIB)/libhelper.a

pebs_latency: pebs_latency.o $(LIB)/libhelper.a

pebs_registers: pebs_registers.o $(LIB)/libhelper.a

offcore_response_mask: offcore_response_mask.o $(LIB)/libhelper.a

uncore_events: uncore_events.o $(LIB)/libhelper.a

uncore_max_cbox: uncore_max_cbox.o $(LIB)/libhelper.a

uncore_group_cbox: uncore_group_cbox.o $(LIB)/libhelper.a

uncore_cpu_cbox: uncore_cpu_cbox.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/x86_intel
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/x86_intel

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
