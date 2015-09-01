include ../../Makefile.globals.inc

LIBNAME = trinity_lib.a

CPPFLAGS += -I./include -I../../include -I.
CFLAGS += -O2 -Wall -g
LDFLAGS +=

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
