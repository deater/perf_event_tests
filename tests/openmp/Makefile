include ../../Makefile.globals.inc

CPPFLAGS += -I../../include
CFLAGS += -Wall -O2 -fopenmp
LDLIBS += -fopenmp
LIB = ../../lib

PROGRAM_LIST = openmp_test openmp_overflow openmp_sample

all: $(PROGRAM_LIST)

openmp_test: openmp_test.o \
		$(LIB)/libhelper.a

openmp_overflow: openmp_overflow.o \
		$(LIB)/libhelper.a

openmp_sample: openmp_sample.o \
		$(LIB)/libhelper.a

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
