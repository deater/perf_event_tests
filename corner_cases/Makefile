CC = gcc
CFLAGS = -Wall -O2 -I../include
LFLAGS =

all:	\
	sampled_notleader_refresh \
	overflow_requires_mmap \
	ioctl_refresh_0 \
	context_switch_user_kernel \
	wrong_size_enospc \
	signal_after_exec

#

../lib/perf_helpers.o:	../lib/perf_helpers.c
	cd ../lib && make

../lib/test_utils.o:	../lib/test_utils.c
	cd ../lib && make

../lib/matrix_multiply.o:	../lib/matrix_multiply.c
	cd ../lib && make

###

overflow_requires_mmap:	overflow_requires_mmap.o \
			../lib/perf_helpers.o \
			../lib/test_utils.o
	$(CC) $(LFLAGS) -o overflow_requires_mmap overflow_requires_mmap.o \
			../lib/perf_helpers.o \
			../lib/test_utils.o

overflow_requires_mmap.o:	overflow_requires_mmap.c
	$(CC) $(CFLAGS) -c overflow_requires_mmap.c

###

sampled_notleader_refresh:	sampled_notleader_refresh.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o
	$(CC) $(LFLAGS) -o sampled_notleader_refresh sampled_notleader_refresh.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o

sampled_notleader_refresh.o:	sampled_notleader_refresh.c
	$(CC) $(CFLAGS) -c sampled_notleader_refresh.c

###

context_switch_user_kernel:	context_switch_user_kernel.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o context_switch_user_kernel context_switch_user_kernel.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o

context_switch_user_kernel.o:	context_switch_user_kernel.c
	$(CC) $(CFLAGS) -c context_switch_user_kernel.c

###

signal_after_exec:	signal_after_exec.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o 

	$(CC) $(LFLAGS) -o signal_after_exec signal_after_exec.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o 

signal_after_exec.o:	signal_after_exec.c
	$(CC) $(CFLAGS) -c signal_after_exec.c


###

wrong_size_enospc:	wrong_size_enospc.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o wrong_size_enospc wrong_size_enospc.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o

wrong_size_enospc.o:	wrong_size_enospc.c
	$(CC) $(CFLAGS) -c wrong_size_enospc.c


###

ioctl_refresh_0:	ioctl_refresh_0.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o ioctl_refresh_0 ioctl_refresh_0.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o

ioctl_refresh_0.o:	ioctl_refresh_0.c
	$(CC) $(CFLAGS) -c ioctl_refresh_0.c


###

clean:	
	rm -f *~ *.o \
	sampled_notleader_refresh \
	overflow_requires_mmap \
	ioctl_refresh_0 \
	context_switch_user_kernel \
	wrong_size_enospc \
	signal_after_exec
