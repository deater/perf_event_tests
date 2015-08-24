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

install: all
	$(INSTALL) -d $(prefix)/warnings
	$(INSTALL) -m755 $(PROGRAM_LIST) $(prefix)/warnings

clean: clean-local
	@- $(RM) $(PROGRAM_LIST)
	@- $(RM) *.o

clean-local:
	@- $(RM) *~
