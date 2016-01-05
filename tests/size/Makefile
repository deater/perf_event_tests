include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	header_mismatch \
	just_right \
	too_big \
	too_little

all: $(PROGRAM_LIST)

header_mismatch: header_mismatch.o $(LIB)/libhelper.a

just_right: just_right.o $(LIB)/libhelper.a

too_big: too_big.o $(LIB)/libhelper.a

too_little: too_little.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/size
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/size

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
