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

eperm: eperm.o $(LIB)/libhelper.a

esrch: esrch.o $(LIB)/libhelper.a

non-existent: non-existent.o $(LIB)/libhelper.a

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
