include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	breakpoint_overflow \
	overflow_large \
	overflow_poll \
	overflow_signal \
	overflow_skid \
	overflow_requires_wakeup \
	simple_overflow_leader \
	simple_overflow_leader_poll \
	simple_overflow_sibling \
	simultaneous_group_overflow \
	simul_oneshot_group_overflow \
	simultaneous_overflow  \
	single_shot_overflow \
	single_shot_w_enable \
	wakeup_events_overflow

all: $(PROGRAM_LIST)

breakpoint_overflow: breakpoint_overflow.o $(LIB)/libhelper.a

overflow_poll: overflow_poll.o $(LIB)/libhelper.a

overflow_signal: overflow_signal.o $(LIB)/libhelper.a

overflow_skid: overflow_skid.o $(LIB)/libhelper.a

overflow_large: overflow_large.o $(LIB)/libhelper.a

simultaneous_group_overflow: simultaneous_group_overflow.o $(LIB)/libhelper.a

simul_oneshot_group_overflow: simul_oneshot_group_overflow.o $(LIB)/libhelper.a

simultaneous_overflow: simultaneous_overflow.o $(LIB)/libhelper.a

overflow_requires_wakeup: overflow_requires_wakeup.o $(LIB)/libhelper.a

simple_overflow_leader: simple_overflow_leader.o $(LIB)/libhelper.a

simple_overflow_leader_poll: simple_overflow_leader_poll.o $(LIB)/libhelper.a

simple_overflow_sibling: simple_overflow_sibling.o $(LIB)/libhelper.a

single_shot_overflow: single_shot_overflow.o $(LIB)/libhelper.a

single_shot_w_enable: single_shot_w_enable.o $(LIB)/libhelper.a

wakeup_events_overflow: wakeup_events_overflow.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/overflow
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/overflow

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
