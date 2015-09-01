include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = socket_pass

all: $(PROGRAM_LIST)

socket_pass: socket_pass.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/socket
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/socket

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
