include ../Makefile.globals.inc

CPPFLAGS += -I../include
CFLAGS += -Wall -O2 -g
CFLAGS_X32 = -mx32

LIBNAME = libhelper.a
LIBNAME_X32 = libhelper.xa

PAPI_FILE_LIST_C := $(wildcard papi*.c)

.SUFFIXES: .xo

.c.xo:
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS) $(CFLAGS_X32)


FILE_LIST_C := $(filter-out $(PAPI_FILE_LIST_C),$(wildcard *.c))
FILE_LIST_C_OBJS := $(FILE_LIST_C:.c=.o)
FILE_LIST_X32_OBJS := $(FILE_LIST_C:.c=.xo)

all: $(LIBNAME)

x32: $(LIBNAME_X32)

$(LIBNAME): $(FILE_LIST_C_OBJS)
	$(AR) r $@ $^
	$(RANLIB) $@

$(LIBNAME_X32): $(FILE_LIST_X32_OBJS)
	$(AR) r $@ $^
	$(RANLIB) $@

clean: clean-local
	@- $(RM) $(LIBNAME) $(LIBNAME_X32)
	@- $(RM) $(FILE_LIST_C_OBJS)
	@- $(RM) *.o *.xo

clean-local:
	@- $(RM) *~
