include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	flags_cgroup \
	flags_fd_no_group \
	flags_fd_output \
	flags_range_check

all: $(PROGRAM_LIST)


flags_cgroup:flags_cgroup.o $(LIB)/libhelper.a

flags_fd_no_group:flags_fd_no_group.o $(LIB)/libhelper.a

flags_fd_output:flags_fd_output.o $(LIB)/libhelper.a

flags_range_check: flags_range_check.o $(LIB)/libhelper.a

clean:
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o
	@- $(RM) *~
