include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	check_paranoid \
	get_cache_info

all: $(PROGRAM_LIST)

check_paranoid: check_paranoid.o $(LIB)/libhelper.a

get_cache_info: get_cache_info.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/utils
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/utils

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
