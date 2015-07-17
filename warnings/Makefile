include ../Makefile.globals.inc

CPPFLAGS += -I../include
CFLAGS += -Wall -O2
LDFLAGS +=

PROGRAM_LIST = \
	core2_nmi_flood \
	kec_2122_warn \
	pec_1076_warn

all: $(PROGRAM_LIST)

core2_nmi_flood: core2_nmi_flood.o

kec_2122_warn: kec_2122_warn.o ../lib/libhelper.a

pec_1076_warn: pec_1076_warn.o ../lib/libhelper.a

clean:
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o
	@- $(RM) *~
