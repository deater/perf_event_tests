include ../Makefile.globals.inc

CPPFLAGS += -I../include
CFLAGS += -Wall -O2
#CFLAGS_X32 = -mx32 -Wall -O2 -I../include

LIBNAME = libhelper.a

PAPI_FILE_LIST_C := $(wildcard papi*.c)

FILE_LIST_C := $(filter-out $(PAPI_FILE_LIST_C),$(wildcard *.c))
FILE_LIST_C_OBJS := $(FILE_LIST_C:.c=.o)

all: $(LIBNAME)

$(LIBNAME): $(FILE_LIST_C_OBJS)
	$(AR) ru $@ $^
	$(RANLIB) $@

clean: clean-local
	@- $(RM) $(LIBNAME)
	@- $(RM) $(FILE_LIST_C_OBJS)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
