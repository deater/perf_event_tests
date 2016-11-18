include ../Makefile.globals.inc

CPPFLAGS += -I../include -I./include -I./trinity_files/include
CFLAGS += -Wall -g
LDFLAGS +=
CFLAGS_X32 += -mx32
LDFLAGS_X32 += -mx32 -static

.SUFFIXES: .xo .xa

.c.xo:
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS) $(CFLAGS_X32)


PROGRAM_LIST = perf_fuzzer replay_log log_to_code filter_log active_events

PROGRAM_LIST_X32 = perf_fuzzer.x32 replay_log.x32

all: $(PROGRAM_LIST)

x32:	$(PROGRAM_LIST_X32)

replay_log: replay_log.o \
		../lib/libhelper.a perf_attr_print.o \
		trinity_files/trinity_lib.a

replay_log.x32: replay_log.xo perf_attr_print.xo
	$(CC) $(LDFLAGS_X32) -o replay_log.x32 replay_log.xo perf_attr_print.xo\
		trinity_files/trinity_lib.xa \
		../lib/libhelper.xa

active_events: active_events.o \
		../lib/libhelper.a perf_attr_print.o

log_to_code: log_to_code.o \
		perf_attr_print.o \
		../lib/libhelper.a

perf_fuzzer: perf_fuzzer.o perf_attr_print.o get_cpuinfo.o \
		filter_address.o filter_tracepoint.o \
		fuzz_access.o fuzz_close.o fuzz_fork.o fuzz_ioctl.o \
		fuzz_million.o fuzz_mmap.o fuzz_open.o fuzz_overflow.o \
		fuzz_poll.o fuzz_prctl.o fuzz_read.o fuzz_write.o \
		fuzzer_random.o fuzzer_stats.o \
		../lib/libhelper.a \
		trinity_files/trinity_lib.a

perf_fuzzer.x32: perf_fuzzer.xo perf_attr_print.xo get_cpuinfo.xo \
		filter_address.xo filter_tracepoint.xo \
		fuzz_access.xo fuzz_close.xo fuzz_fork.xo fuzz_ioctl.xo \
		fuzz_million.xo fuzz_mmap.xo fuzz_open.xo fuzz_overflow.xo \
		fuzz_poll.xo fuzz_prctl.xo fuzz_read.xo fuzz_write.xo \
		fuzzer_random.xo fuzzer_stats.xo \
		trinity_files/trinity_lib.xa

	$(CC) $(LDFLAGS_X32) -o perf_fuzzer.x32 \
		perf_fuzzer.xo perf_attr_print.xo get_cpuinfo.xo \
		fuzz_access.xo fuzz_close.xo fuzz_fork.xo fuzz_ioctl.xo \
		fuzz_million.xo fuzz_mmap.xo fuzz_open.xo fuzz_overflow.xo \
		fuzz_poll.xo fuzz_prctl.xo fuzz_read.xo fuzz_write.xo \
		fuzzer_random.xo fuzzer_stats.xo \
		trinity_files/trinity_lib.xa \
		../lib/libhelper.xa


trinity_files/trinity_lib.a:
	$(MAKE) -C trinity_files

install: all
	$(INSTALL) -d $(prefix)/fuzzer
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/fuzzer

clean: clean-local
	@- $(RM) $(PROGRAM_LIST) $(PROGRAM_LIST_X32)
	$(MAKE) -C trinity_files clean
	@- $(RM) *.o *.xo

clean-local:
	@- $(RM) *~
