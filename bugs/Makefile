CC = gcc
CFLAGS = -Wall -O2 -I../include
LFLAGS =

all:	\
	read_group_attached \
	check_schedulability \
	check_papi_multiplexing \
	sync_read_required \
	nmi_watchdog_group_leader

#

../lib/perf_helpers.o:	../lib/perf_helpers.c
	cd ../lib && make

../lib/test_utils.o:	../lib/test_utils.c
	cd ../lib && make

../lib/matrix_multiply.o:	../lib/matrix_multiply.c
	cd ../lib && make

###

read_group_attached:	read_group_attached.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o read_group_attached read_group_attached.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o

read_group_attached.o:	read_group_attached.c
	$(CC) $(CFLAGS) -c read_group_attached.c

###

check_schedulability:	check_schedulability.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o check_schedulability check_schedulability.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o

check_schedulability.o:	check_schedulability.c
	$(CC) $(CFLAGS) -c check_schedulability.c

###	

check_papi_multiplexing:	check_papi_multiplexing.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o check_papi_multiplexing check_papi_multiplexing.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o

check_papi_multiplexing.o:	check_papi_multiplexing.c
	$(CC) $(CFLAGS) -c check_papi_multiplexing.c

###

sync_read_required:	sync_read_required.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o sync_read_required sync_read_required.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o \
				../lib/matrix_multiply.o

sync_read_required.o:	sync_read_required.c
	$(CC) $(CFLAGS) -c sync_read_required.c

###

nmi_watchdog_group_leader:	nmi_watchdog_group_leader.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o 
	$(CC) $(LFLAGS) -o nmi_watchdog_group_leader nmi_watchdog_group_leader.o \
				../lib/perf_helpers.o \
				../lib/test_utils.o 

nmi_watchdog_group_leader.o:	nmi_watchdog_group_leader.c
	$(CC) $(CFLAGS) -c nmi_watchdog_group_leader.c

###

clean:	
	rm -f *~ *.o \
	read_group_attached \
	check_schedulability \
	check_papi_multiplexing \
	sync_read_required \
	nmi_watchdog_group_leader


