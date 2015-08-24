include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	ioctl_flag_group \
	ioctl_id \
	ioctl_period \
	ioctl_set_bpf \
	ioctl_set_filter \
	ioctl_set_output

all: $(PROGRAM_LIST)

ioctl_flag_group: ioctl_flag_group.o $(LIB)/libhelper.a

ioctl_id: ioctl_id.o $(LIB)/libhelper.a

ioctl_period: ioctl_period.o $(LIB)/libhelper.a

ioctl_set_bpf: ioctl_set_bpf.o $(LIB)/libhelper.a

ioctl_set_filter: ioctl_set_filter.o $(LIB)/libhelper.a

ioctl_set_output: ioctl_set_output.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/ioctl
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/ioctl

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
