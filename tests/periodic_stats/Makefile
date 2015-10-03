include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDLIBS += -lpthread
LIB = ../../lib


PROGRAM_LIST = \
	print_record_sample

all: $(PROGRAM_LIST)

print_record_sample: print_record_sample.o

install: all
	$(INSTALL) -d $(prefix)/tests/attr_fields
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/attr_fields

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
