CC = gcc
CFLAGS = -O2 -Wall -I../include
LFLAGS =
LFLAGS_PTHREAD = -lpthread

PAPI_INCLUDE = -I/usr/local/include
PAPI_LIB = /usr/local/lib/libpapi.a

all:	pe_inherit_memleak pe_task_schedule_panic \
	perf_mmap_close_bug offcore_response_mask \
	oflo_sw_cpu_clock_crash sw_event_config_overflow \
	nmi_soft_lockup \
	tracepoint_fork \
	tracepoint_irq_work_exit \
	arm_validate_event_oops arm_map_hw_event_oops \
	perf_ftrace_event_crash

####

../lib/perf_helpers.o:	../lib/perf_helpers.c
	cd ../lib && make

####

arm_validate_event_oops:	arm_validate_event_oops.o
	$(CC) $(LFLAGS) -o arm_validate_event_oops arm_validate_event_oops.o

arm_validate_event_oops.o:	arm_validate_event_oops.c
	$(CC) $(CFLAGS) -c arm_validate_event_oops.c

####

arm_map_hw_event_oops:	arm_map_hw_event_oops.o
	$(CC) $(LFLAGS) -o arm_map_hw_event_oops arm_map_hw_event_oops.o

arm_map_hw_event_oops.o:	arm_map_hw_event_oops.c
	$(CC) $(CFLAGS) -c arm_map_hw_event_oops.c

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


###

perf_mmap_close_bug:	perf_mmap_close_bug.o \
			../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o perf_mmap_close_bug perf_mmap_close_bug.o \
			../lib/perf_helpers.o

perf_mmap_close_bug.o:	perf_mmap_close_bug.c
	$(CC) $(CFLAGS) -c perf_mmap_close_bug.c


####

offcore_response_mask:	offcore_response_mask.o ../lib/perf_helpers.o ../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o offcore_response_mask offcore_response_mask.o ../lib/perf_helpers.o ../lib/matrix_multiply.o



offcore_response_mask.o:	offcore_response_mask.c
	$(CC) $(CFLAGS) -c offcore_response_mask.c


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

####

nmi_soft_lockup:	nmi_soft_lockup.o
	$(CC) $(LFLAGS) -o nmi_soft_lockup nmi_soft_lockup.o


nmi_soft_lockup.o:	nmi_soft_lockup.c
	$(CC) $(CFLAGS) -c nmi_soft_lockup.c


####

tracepoint_fork:	tracepoint_fork.o ../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o tracepoint_fork tracepoint_fork.o ../lib/perf_helpers.o


tracepoint_fork.o:	tracepoint_fork.c
	$(CC) $(CFLAGS) -c tracepoint_fork.c

####

tracepoint_irq_work_exit:	tracepoint_irq_work_exit.o
	$(CC) $(LFLAGS) -o tracepoint_irq_work_exit tracepoint_irq_work_exit.o


tracepoint_irq_work_exit.o:	tracepoint_irq_work_exit.c
	$(CC) $(CFLAGS) -c tracepoint_irq_work_exit.c


####

perf_ftrace_event_crash:	perf_ftrace_event_crash.o
	$(CC) $(LFLAGS) -o perf_ftrace_event_crash perf_ftrace_event_crash.o

perf_ftrace_event_crash.o:	perf_ftrace_event_crash.c
	$(CC) $(CFLAGS) -c perf_ftrace_event_crash.c

####

clean:	
	rm -f *~ *.o pe_inherit_memleak pe_task_schedule_panic \
		perf_mmap_close_bug offcore_response_mask \
		oflo_sw_cpu_clock_crash sw_event_config_overflow \
		nmi_soft_lockup tracepoint_fork tracepoint_irq_work_exit \
		arm_validate_event_oops arm_map_hw_event_oops \
		perf_ftrace_event_crash


