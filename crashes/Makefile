CC = gcc
CFLAGS = -O2 -Wall
LFLAGS =
LFLAGS_PTHREAD = -lpthread

PAPI_INCLUDE = -I/usr/local/include
PAPI_LIB = /usr/local/lib/libpapi.a

all:	pe_inherit_memleak pe_multiplex_panic mpx-pthread

####

pe_inherit_memleak:	pe_inherit_memleak.o
	$(CC) $(LFLAGS) $(LFLAGS_PTHREAD) -o pe_inherit_memleak pe_inherit_memleak.o

pe_inherit_memleak.o:	pe_inherit_memleak.c
	$(CC) $(CFLAGS) -c pe_inherit_memleak.c


####

pe_multiplex_panic:	pe_multiplex_panic.o
	$(CC) $(LFLAGS) $(LFLAGS_PTHREAD) -o pe_multiplex_panic pe_multiplex_panic.o

pe_multiplex_panic.o:	pe_multiplex_panic.c
	$(CC) $(CFLAGS) -c pe_multiplex_panic.c


####

mpx-pthread:	mpx-pthread.o
	$(CC) $(LFLAGS) $(LFLAGS_PTHREAD) -o mpx-pthread mpx-pthread.o $(PAPI_LIB)

mpx-pthread.o:	mpx-pthread.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c mpx-pthread.c




clean:	
	rm -f *~ *.o pe_inherit_memleak pe_multiplex_panic mpx-pthread


