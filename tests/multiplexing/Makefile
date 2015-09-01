include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	check_multiplexing \
	check_papi_multiplexing \
	old_multiplexing

all: $(PROGRAM_LIST)


check_papi_multiplexing: check_papi_multiplexing.o \
		$(LIB)/libhelper.a

check_multiplexing: check_multiplexing.o \
		$(LIB)/libhelper.a

old_multiplexing: old_multiplexing.o $(LIB)/libhelper.a

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
