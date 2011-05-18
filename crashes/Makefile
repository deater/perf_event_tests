CC = gcc
CFLAGS = -O2 -Wall
LFLAGS =
LFLAGS_PTHREAD = -lpthread

PAPI_INCLUDE = -I/usr/local/include
PAPI_LIB = /usr/local/lib/libpapi.a

all:	pe_inherit_memleak pe_task_schedule_panic

####

pe_inherit_memleak:	pe_inherit_memleak.o
	$(CC) $(LFLAGS) $(LFLAGS_PTHREAD) -o pe_inherit_memleak pe_inherit_memleak.o

pe_inherit_memleak.o:	pe_inherit_memleak.c
	$(CC) $(CFLAGS) -c pe_inherit_memleak.c


####

pe_task_schedule_panic:	pe_task_schedule_panic.o
	$(CC) $(LFLAGS) $(LFLAGS_PTHREAD) -o pe_task_schedule_panic pe_task_schedule_panic.o

pe_task_schedule_panic.o:	pe_task_schedule_panic.c
	$(CC) $(CFLAGS) -c pe_task_schedule_panic.c


####


clean:	
	rm -f *~ *.o pe_inherit_memleak pe_task_schedule_panic


