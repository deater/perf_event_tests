CC = gcc
CFLAGS = -O2 -Wall -I../include
LFLAGS =
LFLAGS_PTHREAD = -lpthread

PAPI_INCLUDE = -I/usr/local/include
PAPI_LIB = /usr/local/lib/libpapi.a

all:	pe_inherit_memleak pe_task_schedule_panic \
	oflo_sw_cpu_clock_crash sw_event_config_overflow

####

../lib/perf_helpers.o:	../lib/perf_helpers.c
	cd ../lib && make

####

pe_inherit_memleak:	pe_inherit_memleak.o ../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o pe_inherit_memleak pe_inherit_memleak.o ../lib/perf_helpers.o $(LFLAGS_PTHREAD)

pe_inherit_memleak.o:	pe_inherit_memleak.c
	$(CC) $(CFLAGS) -c pe_inherit_memleak.c


####

pe_task_schedule_panic:	pe_task_schedule_panic.o ../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o pe_task_schedule_panic pe_task_schedule_panic.o ../lib/perf_helpers.o $(LFLAGS_PTHREAD)

pe_task_schedule_panic.o:	pe_task_schedule_panic.c
	$(CC) $(CFLAGS) -c pe_task_schedule_panic.c


####

oflo_sw_cpu_clock_crash:	oflo_sw_cpu_clock_crash.o ../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o oflo_sw_cpu_clock_crash oflo_sw_cpu_clock_crash.o ../lib/perf_helpers.o


oflo_sw_cpu_clock_crash.o:	oflo_sw_cpu_clock_crash.c
	$(CC) $(CFLAGS) -c oflo_sw_cpu_clock_crash.c

####

sw_event_config_overflow:	sw_event_config_overflow.o ../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o sw_event_config_overflow sw_event_config_overflow.o ../lib/perf_helpers.o


sw_event_config_overflow.o:	sw_event_config_overflow.c
	$(CC) $(CFLAGS) -c sw_event_config_overflow.c


clean:	
	rm -f *~ *.o pe_inherit_memleak pe_task_schedule_panic \
		oflo_sw_cpu_clock_crash sw_event_config_overflow


