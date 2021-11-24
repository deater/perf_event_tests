include ../../Makefile.globals.inc

DIR_LIBPERF = /usr/local/lib/libperf
INCLUDES = -I$(DIR_LIBPERF)/include
RPATH = -Wl,-rpath $(DIR_LIBPERF)/lib64
LPATH = -L$(DIR_LIBPERF)/lib64

CPPFLAGS += -I../../include $(INCLUDES)
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LDLIBS += -lpthread
LIB = ../../lib

PROGRAM_LIST = \

all: $(PROGRAM_LIST)

rdpmc_lib.o: rdpmc_lib.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c rdpmc_lib.c

install: all
	$(INSTALL) -d $(prefix)/tests/rdpmc_libperf
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/rdpmc_libperf

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
