CC = gcc
CFLAGS = -Wall -O2 -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	check_reset_mpx \
	context_switch_user_kernel \
	hw_sw_mix \
	ioctl_refresh_0 \
	max_multiplex \
	multiple_active \
	overflow_requires_mmap \
	reset_leader \
	sampled_notleader_refresh \
	signal_after_exec \
	sw_start_leader \
	wrong_size_enospc \


#

$(LIB)/perf_helpers.o:	$(LIB)/perf_helpers.c
	cd $(LIB) && make

$(LIB)/test_utils.o:	$(LIB)/test_utils.c
	cd $(LIB) && make

$(LIB)/matrix_multiply.o:	$(LIB)/matrix_multiply.c
	cd $(LIB) && make

###

overflow_requires_mmap:	overflow_requires_mmap.o \
			$(LIB)/perf_helpers.o \
			$(LIB)/test_utils.o
	$(CC) $(LFLAGS) -o overflow_requires_mmap overflow_requires_mmap.o \
			$(LIB)/perf_helpers.o \
			$(LIB)/test_utils.o

overflow_requires_mmap.o:	overflow_requires_mmap.c
	$(CC) $(CFLAGS) -c overflow_requires_mmap.c

###

sampled_notleader_refresh:	sampled_notleader_refresh.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o
	$(CC) $(LFLAGS) -o sampled_notleader_refresh sampled_notleader_refresh.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o

sampled_notleader_refresh.o:	sampled_notleader_refresh.c
	$(CC) $(CFLAGS) -c sampled_notleader_refresh.c

###

reset_leader:	reset_leader.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o reset_leader reset_leader.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o

reset_leader.o:	reset_leader.c
	$(CC) $(CFLAGS) -c reset_leader.c

###

check_reset_mpx:	check_reset_mpx.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o check_reset_mpx check_reset_mpx.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o

check_reset_mpx.o:	check_reset_mpx.c
	$(CC) $(CFLAGS) -c check_reset_mpx.c

###


context_switch_user_kernel:	context_switch_user_kernel.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o context_switch_user_kernel context_switch_user_kernel.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o

context_switch_user_kernel.o:	context_switch_user_kernel.c
	$(CC) $(CFLAGS) -c context_switch_user_kernel.c

###

multiple_active:	multiple_active.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o multiple_active multiple_active.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o

multiple_active.o:	multiple_active.c
	$(CC) $(CFLAGS) -c multiple_active.c

###

max_multiplex:	max_multiplex.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o max_multiplex max_multiplex.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o

max_multiplex.o:	max_multiplex.c
	$(CC) $(CFLAGS) -c max_multiplex.c

###



signal_after_exec:	signal_after_exec.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o 

	$(CC) $(LFLAGS) -o signal_after_exec signal_after_exec.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o 

signal_after_exec.o:	signal_after_exec.c
	$(CC) $(CFLAGS) -c signal_after_exec.c

###

hw_sw_mix:	hw_sw_mix.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o hw_sw_mix hw_sw_mix.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o

hw_sw_mix.o:	hw_sw_mix.c
	$(CC) $(CFLAGS) -c hw_sw_mix.c


###

sw_start_leader:	sw_start_leader.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o sw_start_leader sw_start_leader.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o

sw_start_leader.o:	sw_start_leader.c
	$(CC) $(CFLAGS) -c sw_start_leader.c



###

wrong_size_enospc:	wrong_size_enospc.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o wrong_size_enospc wrong_size_enospc.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o

wrong_size_enospc.o:	wrong_size_enospc.c
	$(CC) $(CFLAGS) -c wrong_size_enospc.c


###

ioctl_refresh_0:	ioctl_refresh_0.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o ioctl_refresh_0 ioctl_refresh_0.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o

ioctl_refresh_0.o:	ioctl_refresh_0.c
	$(CC) $(CFLAGS) -c ioctl_refresh_0.c


###

clean:	
	rm -f *~ *.o \
	check_reset_mpx \
	context_switch_user_kernel \
	hw_sw_mix \
	ioctl_refresh_0 \
	max_multiplex \
	multiple_active \
	overflow_requires_mmap \
	reset_leader \
	sampled_notleader_refresh \
	sw_start_leader \
	signal_after_exec \
	wrong_size_enospc
