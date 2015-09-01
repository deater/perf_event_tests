include ../Makefile.globals.inc

CPPFLAGS += -I../include -I./include -I./trinity_files/include
CFLAGS += -Wall -g
LDFLAGS +=

PROGRAM_LIST = perf_fuzzer replay_log log_to_code filter_log active_events

all: $(PROGRAM_LIST)

replay_log: replay_log.o \
		../lib/libhelper.a perf_attr_print.o \
		trinity_files/trinity_lib.a

active_events: active_events.o \
		../lib/libhelper.a perf_attr_print.o

log_to_code: log_to_code.o \
		perf_attr_print.o \
		../lib/libhelper.a

perf_fuzzer: perf_fuzzer.o perf_attr_print.o get_cpuinfo.o \
		fuzz_access.o fuzz_close.o fuzz_fork.o fuzz_ioctl.o \
		fuzz_million.o fuzz_mmap.o fuzz_open.o fuzz_overflow.o \
		fuzz_poll.o fuzz_prctl.o fuzz_read.o fuzz_write.o \
		fuzzer_random.o fuzzer_stats.o \
		../lib/libhelper.a \
		trinity_files/trinity_lib.a

trinity_files/trinity_lib.a:
	$(MAKE) -C trinity_files

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	$(MAKE) -C trinity_files clean
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
