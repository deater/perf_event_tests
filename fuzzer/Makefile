CC = gcc
CFLAGS = -O2 -Wall
LFLAGS = 

all:	perf_fuzzer

perf_fuzzer:	perf_fuzzer.o \
		../lib/perf_helpers.o ../lib/instructions_testcode.o \
		fds.o interesting-numbers.o perf_event_open.o pids.o \
		random.o random-length.o
	$(CC) $(LFLAGS) -o perf_fuzzer perf_fuzzer.o \
		../lib/perf_helpers.o ../lib/instructions_testcode.o \
		fds.o interesting-numbers.o perf_event_open.o pids.o \
		random.o random-length.o

perf_fuzzer.o:	perf_fuzzer.c
	$(CC) $(CFLAGS) -c perf_fuzzer.c


perf_event_open.o:	perf_event_open.c
	$(CC) $(CFLAGS) -c perf_event_open.c

fds.o:	fds.c
	$(CC) $(CFLAGS) -c fds.c

interesting-numbers.o:	interesting-numbers.c
	$(CC) $(CFLAGS) -c interesting-numbers.c

pids.o:		pids.c
	$(CC) $(CFLAGS) -c pids.c

random.o:	random.c
	$(CC) $(CFLAGS) -c random.c

random-length.o:	random-length.c
	$(CC) $(CFLAGS) -c random-length.c

clean:	
	rm -f *~ *.o perf_fuzzer

