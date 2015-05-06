CC = gcc
AR = ar

CFLAGS = -O2 -Wall -g -I./include -I../../include -I.
LFLAGS = 

CFLAGS_X32 = -mx32 -O2 -Wall -g -I./include -I../../include
LFLAGS_X32 = -mx32

all:	trinity_lib.a

x32:	trinity_lib.x32.a

#####

trinity_lib.a:	 fds.o generic-sanitise.o \
		interesting-numbers.o log.o \
		perf_event_open.o pids.o \
		random.o random-address.o random-length.o \
		shm.o utils.o unicode.o
	$(AR) rv trinity_lib.a \
		fds.o \
		generic-sanitise.o \
		interesting-numbers.o \
		log.o \
		perf_event_open.o \
		pids.o \
		random.o \
		random-address.o \
		random-length.o \
		shm.o \
		utils.o \
		unicode.o

trinity_lib.x32.a:	 fds.x32.o generic-sanitise.x32.o \
		interesting-numbers.x32.o log.x32.o \
		perf_event_open.x32.o pids.x32.o \
		random.x32.o random-address.x32.o random-length.x32.o \
		shm.x32.o utils.x32.o unicode.x32.o
	$(AR) rv trinity_lib.x32.a \
		fds.x32.o generic-sanitise.x32.o \
		interesting-numbers.x32.o log.x32.o \
		perf_event_open.x32.o pids.x32.o \
		random.x32.o random-address.x32.o random-length.x32.o \
		shm.x32.o utils.x32.o unicode.x32.o

#####

perf_event_open.o:	perf_event_open.c
	$(CC) $(CFLAGS) -c perf_event_open.c

perf_event_open.x32.o:	perf_event_open.c
	$(CC) $(CFLAGS_X32) -o perf_event_open.x32.o -c perf_event_open.c

#####

fds.o:	fds.c
	$(CC) $(CFLAGS) -c fds.c

fds.x32.o:	fds.c
	$(CC) $(CFLAGS_X32) -o fds.x32.o -c fds.c

#####

generic-sanitise.o:	generic-sanitise.c
	$(CC) $(CFLAGS) -c generic-sanitise.c

generic-sanitise.x32.o:	generic-sanitise.c
	$(CC) $(CFLAGS_X32) -o generic-sanitise.x32.o -c generic-sanitise.c

#####

interesting-numbers.o:	interesting-numbers.c
	$(CC) $(CFLAGS) -c interesting-numbers.c

interesting-numbers.x32.o:	interesting-numbers.c
	$(CC) $(CFLAGS_X32) -o interesting-numbers.x32.o -c interesting-numbers.c

#####

log.o:		log.c
	$(CC) $(CFLAGS) -c log.c

log.x32.o:		log.c
	$(CC) $(CFLAGS_X32) -o log.x32.o -c log.c

#####

pids.o:		pids.c
	$(CC) $(CFLAGS) -c pids.c

pids.x32.o:		pids.c
	$(CC) $(CFLAGS_X32) -o pids.x32.o -c pids.c

#####

random.o:	random.c
	$(CC) $(CFLAGS) -c random.c

random.x32.o:	random.c
	$(CC) $(CFLAGS_X32) -o random.x32.o -c random.c

#####

random-address.o:	random-address.c
	$(CC) $(CFLAGS) -c random-address.c

random-address.x32.o:	random-address.c
	$(CC) $(CFLAGS_X32) -o random-address.x32.o -c random-address.c

#####

random-length.o:	random-length.c
	$(CC) $(CFLAGS) -c random-length.c

random-length.x32.o:	random-length.c
	$(CC) $(CFLAGS_X32) -o random-length.x32.o -c random-length.c

#####

shm.o:		shm.c
	$(CC) $(CFLAGS) -c shm.c

shm.x32.o:		shm.c
	$(CC) $(CFLAGS_X32) -o shm.x32.o -c shm.c

######

tables.o:	tables.c
	$(CC) $(CFLAGS) -c tables.c

tables.x32.o:	tables.c
	$(CC) $(CFLAGS_X32) -o tables.x32.o -c tables.c

######

tables-uniarch.o:	tables-uniarch.c
	$(CC) $(CFLAGS) -c tables-uniarch.c

tables-uniarch.x32.o:	tables-uniarch.c
	$(CC) $(CFLAGS_X32) -o tables-uniarch.x32.o -c tables-uniarch.c

######

unicode.o:	unicode.c
	$(CC) $(CFLAGS) -c unicode.c

unicode.x32.o:	unicode.c
	$(CC) $(CFLAGS_X32) -o unicode.x32.o -c unicode.c

######

utils.o:	utils.c
	$(CC) $(CFLAGS) -c utils.c

utils.x32.o:	utils.c
	$(CC) $(CFLAGS_X32) -o utils.x32.o -c utils.c

######

clean:	
	rm -f *~ *.o *.a

