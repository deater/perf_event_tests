include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	lost_record_sample \
	mmap_multiple \
	multiple_mmap_sizes \
	overwrite_mmap \
	print_record_sample \
	record_comm \
	record_comm_exec \
	record_fork \
	record_mmap \
	record_mmap2 \
	record_mmap_data \
	sample_branch_stack \
	sample_data_src \
	sample_regs_intr \
	sample_regs_user \
	sample_stack_user \
	sample_weight \
	validate_record_sample

all: $(PROGRAM_LIST)

lost_record_sample: lost_record_sample.o \
		$(LIB)/libhelper.a

mmap_multiple: mmap_multiple.o \
		$(LIB)/libhelper.a

multiple_mmap_sizes: multiple_mmap_sizes.o \
		$(LIB)/libhelper.a

overwrite_mmap: overwrite_mmap.o \
		$(LIB)/libhelper.a

print_record_sample: print_record_sample.o \
		$(LIB)/libhelper.a

record_comm: record_comm.o \
		$(LIB)/libhelper.a

record_comm_exec: record_comm_exec.o \
		$(LIB)/libhelper.a

record_fork: record_fork.o \
		$(LIB)/libhelper.a

record_mmap: record_mmap.o \
		$(LIB)/libhelper.a

record_mmap2: record_mmap2.o \
		$(LIB)/libhelper.a

record_mmap_data: record_mmap_data.o \
		$(LIB)/libhelper.a

validate_record_sample: validate_record_sample.o \
		$(LIB)/libhelper.a

sample_branch_stack: sample_branch_stack.o \
		$(LIB)/libhelper.a

sample_data_src: sample_data_src.o \
		$(LIB)/libhelper.a

sample_regs_intr: sample_regs_intr.o \
		$(LIB)/libhelper.a

sample_regs_user: sample_regs_user.o \
		$(LIB)/libhelper.a

sample_stack_user: sample_stack_user.o \
		$(LIB)/libhelper.a

sample_weight: sample_weight.o \
		$(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/record_sample
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/record_sample

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
