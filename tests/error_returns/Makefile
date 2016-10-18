include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -g
LDFLAGS +=
LIB = ../../lib

PROGRAM_LIST = \
	e2big \
	eacces \
	ebadf \
	efault \
	einval \
	emfile \
	enoent \
	enospc \
	eopnotsupp \
	eoverflow \
	eperm \
	esrch \
	non-existent

all: $(PROGRAM_LIST)

e2big: e2big.o $(LIB)/libhelper.a

eacces: eacces.o $(LIB)/libhelper.a

ebadf: ebadf.o $(LIB)/libhelper.a

efault: efault.o $(LIB)/libhelper.a

einval: einval.o $(LIB)/libhelper.a

emfile: emfile.o $(LIB)/libhelper.a

enoent: enoent.o $(LIB)/libhelper.a

enospc: enospc.o $(LIB)/libhelper.a

eopnotsupp: eopnotsupp.o $(LIB)/libhelper.a

eoverflow: eoverflow.o $(LIB)/libhelper.a

eperm: eperm.o $(LIB)/libhelper.a

esrch: esrch.o $(LIB)/libhelper.a

non-existent: non-existent.o $(LIB)/libhelper.a

install: all
	$(INSTALL) -d $(prefix)/tests/error_returns
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/tests/error_returns

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
