CC = gcc
AR = ar
CFLAGS = -O2 -Wall -g -I./include -I../../include
LFLAGS = 

all:	trinity_lib.a

trinity_lib.a:	 fds.o interesting-numbers.o perf_event_open.o pids.o \
		random.o random-address.o random-length.o random-page.o \
		maps-static.o unicode.o
	$(AR) rv trinity_lib.a \
		fds.o interesting-numbers.o perf_event_open.o pids.o \
		random.o random-address.o random-length.o random-page.o \
		maps-static.o unicode.o

perf_event_open.o:	perf_event_open.c
	$(CC) $(CFLAGS) -c perf_event_open.c

fds.o:	fds.c
	$(CC) $(CFLAGS) -c fds.c

interesting-numbers.o:	interesting-numbers.c
	$(CC) $(CFLAGS) -c interesting-numbers.c

maps-static.o:	maps-static.c
	$(CC) $(CFLAGS) -c maps-static.c

pids.o:		pids.c
	$(CC) $(CFLAGS) -c pids.c

random.o:	random.c
	$(CC) $(CFLAGS) -c random.c

random-address.o:	random-address.c
	$(CC) $(CFLAGS) -c random-address.c

random-length.o:	random-length.c
	$(CC) $(CFLAGS) -c random-length.c

random-page.o:	random-page.c
	$(CC) $(CFLAGS) -c random-page.c

unicode.o:	unicode.c
	$(CC) $(CFLAGS) -c unicode.c

clean:	
	rm -f *~ *.o *.a

