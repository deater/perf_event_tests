include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS += -lpthread
LIB = ../../lib

PROGRAM_LIST = \
	disabled \
	enable_on_exec \
	exclude_guest \
	exclude_host \
	exclude_hv \
	exclude_kernel \
	exclude_user \
	exclusive \
	format_id_support \
	inherit \
	inherit_stat \
	read_format_range_check \
	sample_type_range_check

all: $(PROGRAM_LIST)

inherit: inherit.o $(LIB)/libhelper.a

inherit_stat: inherit_stat.o $(LIB)/libhelper.a

disabled:disabled.o $(LIB)/libhelper.a

exclude_user:exclude_user.o $(LIB)/libhelper.a

exclude_kernel: exclude_kernel.o $(LIB)/libhelper.a

exclude_host: exclude_host.o $(LIB)/libhelper.a

exclude_guest: exclude_guest.o $(LIB)/libhelper.a

exclude_hv: exclude_hv.o $(LIB)/libhelper.a

enable_on_exec: enable_on_exec.o $(LIB)/libhelper.a

exclusive: exclusive.o $(LIB)/libhelper.a


format_id_support: format_id_support.o $(LIB)/libhelper.a

read_format_range_check:	read_format_range_check.o $(LIB)/libhelper.a

sample_type_range_check:	sample_type_range_check.o $(LIB)/libhelper.a

clean:
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o
	@- $(RM) *~
