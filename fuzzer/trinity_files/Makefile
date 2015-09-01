include ../../Makefile.globals.inc

LIBNAME = trinity_lib.a

LIBNAME_X32 = trinity_lib.xa

CPPFLAGS += -I./include -I../../include -I.
CFLAGS += -O2 -Wall -g
CFLAGS_X32 += -mx32
LDFLAGS +=

FILE_LIST_C := $(filter-out $(PAPI_FILE_LIST_C),$(wildcard *.c))
FILE_LIST_C_OBJS := $(FILE_LIST_C:.c=.o)
FILE_LIST_X32_OBJS := $(FILE_LIST_C:.c=.xo)

.SUFFIXES: .xo .x32

.c.xo:
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS) $(CFLAGS_X32)


all: $(LIBNAME)

x32:	$(LIBNAME_X32)

$(LIBNAME): $(FILE_LIST_C_OBJS)
	$(AR) ru $@ $^
	$(RANLIB) $@

$(LIBNAME_X32): $(FILE_LIST_X32_OBJS)
	$(AR) ru $@ $^
	$(RANLIB) $@

clean: clean-local
	@- $(RM) $(LIBNAME) $(LIBNAME_X32)
	@- $(RM) $(FILE_LIST_C_OBJS)
	@- $(RM) *.o *.xo

clean-local:
	@- $(RM) *~
