include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST =	\
	check_reset_mpx \
	context_switch_user_kernel \
	huge_events_start \
	huge_group_start \
	hw_sw_mix \
	ioctl_refresh_0 \
	max_multiplex \
	max_num_events \
	multiple_active \
	overflow_requires_mmap \
	reset_leader \
	sampled_notleader_refresh \
	signal_after_close \
	signal_after_exec \
	sw_start_leader \
	wrap \
	wrong_size_enospc

all: $(PROGRAM_LIST)

check_reset_mpx: check_reset_mpx.o \
		$(LIB)/libhelper.a

context_switch_user_kernel: context_switch_user_kernel.o \
		$(LIB)/libhelper.a

huge_events_start: huge_events_start.o \
		$(LIB)/libhelper.a

huge_group_start: huge_group_start.o \
		$(LIB)/libhelper.a

hw_sw_mix: hw_sw_mix.o \
		$(LIB)/libhelper.a

ioctl_refresh_0: ioctl_refresh_0.o \
		$(LIB)/libhelper.a

max_multiplex: max_multiplex.o \
		$(LIB)/libhelper.a

max_num_events: max_num_events.o \
		$(LIB)/libhelper.a

multiple_active: multiple_active.o \
		$(LIB)/libhelper.a

overflow_requires_mmap: overflow_requires_mmap.o \
		$(LIB)/libhelper.a

reset_leader: reset_leader.o \
		$(LIB)/libhelper.a

sampled_notleader_refresh: sampled_notleader_refresh.o \
		$(LIB)/libhelper.a

signal_after_close: signal_after_close.o \
		$(LIB)/libhelper.a

signal_after_exec: signal_after_exec.o \
		$(LIB)/libhelper.a

sw_start_leader: sw_start_leader.o \
		$(LIB)/libhelper.a

wrap:	wrap.o \
		$(LIB)/libhelper.a

wrong_size_enospc: wrong_size_enospc.o \
		$(LIB)/libhelper.a


install: all
	$(INSTALL) -d $(prefix)/tests/corner_cases
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/corner_cases

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
