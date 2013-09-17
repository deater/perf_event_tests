CC = gcc
CFLAGS = -Wall -O2 -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	check_multiplexing \
	check_papi_multiplexing \
	check_schedulability \
	nmi_watchdog_group_leader \
	read_group_attached \
	sync_read_required \


#

$(LIB)/perf_helpers.o:	$(LIB)/perf_helpers.c
	cd $(LIB) && make

$(LIB)/test_utils.o:	$(LIB)/test_utils.c
	cd $(LIB) && make

$(LIB)/matrix_multiply.o:	$(LIB)/matrix_multiply.c
	cd $(LIB) && make

###

read_group_attached:	read_group_attached.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o read_group_attached read_group_attached.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o

read_group_attached.o:	read_group_attached.c
	$(CC) $(CFLAGS) -c read_group_attached.c

###

check_schedulability:	check_schedulability.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o check_schedulability check_schedulability.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o

check_schedulability.o:	check_schedulability.c
	$(CC) $(CFLAGS) -c check_schedulability.c

###	

check_papi_multiplexing:	check_papi_multiplexing.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o check_papi_multiplexing check_papi_multiplexing.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o

check_papi_multiplexing.o:	check_papi_multiplexing.c
	$(CC) $(CFLAGS) -c check_papi_multiplexing.c

###	

check_multiplexing:	check_multiplexing.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o check_multiplexing check_multiplexing.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o

check_multiplexing.o:	check_multiplexing.c
	$(CC) $(CFLAGS) -c check_multiplexing.c


###

sync_read_required:	sync_read_required.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o sync_read_required sync_read_required.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o

sync_read_required.o:	sync_read_required.c
	$(CC) $(CFLAGS) -c sync_read_required.c

###

nmi_watchdog_group_leader:	nmi_watchdog_group_leader.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o 
	$(CC) $(LFLAGS) -o nmi_watchdog_group_leader nmi_watchdog_group_leader.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o 

nmi_watchdog_group_leader.o:	nmi_watchdog_group_leader.c
	$(CC) $(CFLAGS) -c nmi_watchdog_group_leader.c

####

clean:	
	rm -f *~ *.o \
	check_multiplexing \
	check_papi_multiplexing \
	check_schedulability \
	nmi_watchdog_group_leader \
	read_group_attached \
	sync_read_required \



