include ../Makefile.globals.inc

CPPFLAGS +=
CFLAGS += -Wall -g -O2
LDFLAGS +=
CFLAGS_X32 += -mx32
LDFLAGS_X32 += -mx32 -static

.SUFFIXES: .xo .xa

.c.xo:
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS) $(CFLAGS_X32)


PROGRAM_LIST = perf_tool_fuzzer

all: $(PROGRAM_LIST)

perf_tool_fuzzer: perf_tool_fuzzer.o

install: all
	$(INSTALL) -d $(prefix)/fuzzer
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/fuzzer

clean: clean-local
	@- $(RM) $(PROGRAM_LIST) $(PROGRAM_LIST_X32)
	@- $(RM) *.o *.xo

clean-local:
	@- $(RM) *~
