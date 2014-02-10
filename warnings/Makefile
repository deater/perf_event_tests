CC = gcc
CFLAGS = -O2 -Wall -I../include 
LFLAGS = 
LFLAGS_PTHREAD = -lpthread

all:	core2_nmi_flood kec_2122_warn pec_1076_warn


####

../lib/perf_helpers.o:	../lib/perf_helpers.c
	cd ../lib && make

####

core2_nmi_flood:	core2_nmi_flood.o
	$(CC) $(LFLAGS) -o core2_nmi_flood core2_nmi_flood.o

core2_nmi_flood.o:	core2_nmi_flood.c
	$(CC) $(CFLAGS) -c core2_nmi_flood.c

####

kec_2122_warn:	kec_2122_warn.o
	$(CC) $(LFLAGS) -o kec_2122_warn kec_2122_warn.o ../lib/perf_helpers.o $(LFLAGS)

kec_2122_warn.c:	kec_2122_warn.c
	$(CC) $(CFLAGS) -c kec_2122_warn.c

####

pec_1076_warn:	pec_1076_warn.o ../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o pec_1076_warn pec_1076_warn.o

pec_1076_warn.o:	pec_1076_warn.c
	$(CC) $(CFLAGS) -c pec_1076_warn.c

####

clean:	
	rm -f *~ *.o \
		core2_nmi_flood kec_2122_warn pec_1076_warn



