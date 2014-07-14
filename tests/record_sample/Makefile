CC = gcc
CFLAGS = -Wall -O2 -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	lost_record_sample \
	multiple_mmap_sizes \
	overwrite_mmap \
	print_record_sample \
	sample_branch_stack \
	sample_regs_user \
	sample_stack_user \
	sample_weight \
	validate_record_sample

###

$(LIB)/test_utils.o:	$(LIB)/test_utils.c
	cd $(LIB) && make

$(LIB)/branches_testcode.o:	$(LIB)/branches_testcode.c
	cd $(LIB) && make

$(LIB)/matrix_multiply.o:	$(LIB)/matrix_multiply.c
	cd $(LIB) && make

$(LIB)/parse_record.o:	$(LIB)/parse_record.c
	cd $(LIB) && make

$(LIB)/perf_helpers.o:	$(LIB)/perf_helpers.c
	cd $(LIB) && make

$(LIB)/detect_cache.o:	$(LIB)/detect_cache.c
	cd $(LIB) && make

###

lost_record_sample:	lost_record_sample.o \
					$(LIB)/parse_record.o \
					$(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o lost_record_sample \
				lost_record_sample.o \
				$(LIB)/parse_record.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

lost_record_sample.o:	lost_record_sample.c
	$(CC) $(CFLAGS) -c lost_record_sample.c


###

multiple_mmap_sizes:	multiple_mmap_sizes.o \
					$(LIB)/parse_record.o \
					$(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o multiple_mmap_sizes \
				multiple_mmap_sizes.o \
				$(LIB)/parse_record.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

multiple_mmap_sizes.o:	multiple_mmap_sizes.c
	$(CC) $(CFLAGS) -c multiple_mmap_sizes.c

###

overwrite_mmap:	overwrite_mmap.o \
					$(LIB)/parse_record.o \
					$(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o overwrite_mmap \
				overwrite_mmap.o \
				$(LIB)/parse_record.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

overwrite_mmap.o:	overwrite_mmap.c
	$(CC) $(CFLAGS) -c overwrite_mmap.c

###

print_record_sample:	print_record_sample.o \
					$(LIB)/parse_record.o \
					$(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o print_record_sample \
				print_record_sample.o \
				$(LIB)/parse_record.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

print_record_sample.o:	print_record_sample.c
	$(CC) $(CFLAGS) -c print_record_sample.c

###

validate_record_sample:	validate_record_sample.o \
					$(LIB)/parse_record.o \
					$(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o validate_record_sample \
				validate_record_sample.o \
				$(LIB)/parse_record.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

validate_record_sample.o:	validate_record_sample.c
	$(CC) $(CFLAGS) -c validate_record_sample.c


###

sample_branch_stack:	sample_branch_stack.o \
					$(LIB)/parse_record.o \
					$(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o sample_branch_stack \
				sample_branch_stack.o \
				$(LIB)/parse_record.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

sample_branch_stack.o:	sample_branch_stack.c
	$(CC) $(CFLAGS) -c sample_branch_stack.c



###

sample_regs_user:	sample_regs_user.o \
					$(LIB)/parse_record.o \
					$(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o sample_regs_user \
				sample_regs_user.o \
				$(LIB)/parse_record.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

sample_regs_user.o:	sample_regs_user.c
	$(CC) $(CFLAGS) -c sample_regs_user.c

###

sample_stack_user:	sample_stack_user.o \
					$(LIB)/parse_record.o \
					$(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o sample_stack_user \
				sample_stack_user.o \
				$(LIB)/parse_record.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

sample_stack_user.o:	sample_stack_user.c
	$(CC) $(CFLAGS) -c sample_stack_user.c

###

sample_weight:	sample_weight.o \
					$(LIB)/parse_record.o \
					$(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o sample_weight \
				sample_weight.o \
				$(LIB)/parse_record.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

sample_weight.o:	sample_weight.c
	$(CC) $(CFLAGS) -c sample_weight.c


###

clean:	
	rm -f *~ *.o \
	lost_record_sample \
	multiple_mmap_sizes \
	overwrite_mmap \
	print_record_sample \
	sample_branch_stack \
	sample_regs_user \
	sample_stack_user \
	sample_weight \
	validate_record_sample

