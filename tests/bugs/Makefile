include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS = -Wall -O2
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	check_schedulability \
	nmi_watchdog_group_leader \
	read_group_attached \
	sync_read_required

all: $(PROGRAM_LIST)

read_group_attached: read_group_attached.o \
				$(LIB)/libhelper.a

check_schedulability: check_schedulability.o \
				$(LIB)/libhelper.a

sync_read_required: sync_read_required.o \
				$(LIB)/libhelper.a

nmi_watchdog_group_leader: nmi_watchdog_group_leader.o \
				$(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/bugs
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/bugs

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
