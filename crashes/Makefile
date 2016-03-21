include ../Makefile.globals.inc

CPPFLAGS += -I../include
CFLAGS += -O2 -Wall
LDLIBS += -lpthread

PAPI_INCLUDE = -I/usr/local/include
PAPI_LIB = /usr/local/lib/libpapi.a

PROGRAM_LIST = pe_inherit_memleak pe_task_schedule_panic \
	perf_mmap_close_bug offcore_response_mask \
	oflo_sw_cpu_clock_crash sw_event_config_overflow \
	nmi_soft_lockup \
	tracepoint_irq_work_exit \
	arm_validate_event_oops arm_map_hw_event_oops \
	perf_ftrace_event_crash \
	css_cgroup_crash \
	rapl_crash

all: $(PROGRAM_LIST)

arm_validate_event_oops: arm_validate_event_oops.o

arm_map_hw_event_oops: arm_map_hw_event_oops.o

pe_inherit_memleak: pe_inherit_memleak.o ../lib/libhelper.a

pe_task_schedule_panic: pe_task_schedule_panic.o ../lib/libhelper.a

perf_mmap_close_bug: perf_mmap_close_bug.o ../lib/libhelper.a

offcore_response_mask: offcore_response_mask.o ../lib/libhelper.a

oflo_sw_cpu_clock_crash: oflo_sw_cpu_clock_crash.o ../lib/libhelper.a

sw_event_config_overflow: sw_event_config_overflow.o ../lib/libhelper.a

nmi_soft_lockup: nmi_soft_lockup.o

tracepoint_irq_work_exit: tracepoint_irq_work_exit.o

perf_ftrace_event_crash: perf_ftrace_event_crash.o

css_cgroup_crash: css_cgroup_crash.o ../lib/libhelper.a

rapl_crash: rapl_crash.o ../lib/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/crashes
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/crashes

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
