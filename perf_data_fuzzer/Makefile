include ../Makefile.globals.inc

CPPFLAGS += -I./trinity_files/include
CFLAGS += -Wall -g -O2
LDFLAGS +=
CFLAGS_X32 += -mx32
LDFLAGS_X32 += -mx32 -static

.SUFFIXES: .xo .xa

.c.xo:
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS) $(CFLAGS_X32)


PROGRAM_LIST = perf_data_fuzzer perf_data_dump

all: $(PROGRAM_LIST)

perf_data_dump: perf_data_dump.o parse_record.o data_read.o

perf_data_fuzzer: perf_data_fuzzer.o create_perf_data.o \
		trinity_files/trinity_lib.a


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
