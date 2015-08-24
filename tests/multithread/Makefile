include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	fork_then_read \
	lots_of_forks \
	open_fork_close_kill

all: $(PROGRAM_LIST)

fork_then_read: fork_then_read.o \
		$(LIB)/libhelper.a

lots_of_forks: lots_of_forks.o \
		$(LIB)/libhelper.a

open_fork_close_kill: open_fork_close_kill.o \
		$(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/multithread
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/multithread

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
