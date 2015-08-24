include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = check_constraints

all: $(PROGRAM_LIST)

check_constraints: check_constraints.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/constraints
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/constraints

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
