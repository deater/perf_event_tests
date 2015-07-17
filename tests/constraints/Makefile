include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = check_constraints

all: $(PROGRAM_LIST)

check_constraints: check_constraints.o $(LIB)/libhelper.a

clean:
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o
	@- $(RM) *~
