CC = gcc
CFLAGS = -Wall -O2 -g -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	flags_cgroup \
	flags_fd_no_group \
	flags_fd_output \
	flags_range_check


###

$(LIB)/test_utils.o:	$(LIB)/test_utils.c
	cd $(LIB) && make

$(LIB)/branches_testcode.o:	$(LIB)/branches_testcode.c
	cd $(LIB) && make

$(LIB)/matrix_multiply.o:	$(LIB)/matrix_multiply.c
	cd $(LIB) && make

$(LIB)/perf_helpers.o:	$(LIB)/perf_helpers.c
	cd $(LIB) && make

$(LIB)/detect_cache.o:	$(LIB)/detect_cache.c
	cd $(LIB) && make

###

flags_cgroup:	flags_cgroup.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o flags_cgroup flags_cgroup.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

flags_cgroup.o:	flags_cgroup.c
	$(CC) $(CFLAGS) -c flags_cgroup.c

###

flags_fd_no_group:	flags_fd_no_group.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o flags_fd_no_group flags_fd_no_group.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

flags_fd_no_group.o:	flags_fd_no_group.c
	$(CC) $(CFLAGS) -c flags_fd_no_group.c

###

flags_fd_output:	flags_fd_output.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o flags_fd_output flags_fd_output.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

flags_fd_output.o:	flags_fd_output.c
	$(CC) $(CFLAGS) -c flags_fd_output.c

###

flags_range_check:	flags_range_check.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o flags_range_check flags_range_check.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

flags_range_check.o:	flags_range_check.c
	$(CC) $(CFLAGS) -c flags_range_check.c

###


clean:	
	rm -f *~ *.o \
	flags_cgroup \
	flags_fd_no_group \
	flags_fd_output \
	flags_range_check 
