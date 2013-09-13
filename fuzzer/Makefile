CC = gcc
CFLAGS = -O2 -Wall -g -I../include -I./include
LFLAGS = 

all:	perf_fuzzer replay_log log_to_code filter_log

perf_attr_print.o:	perf_attr_print.c perf_attr_print.h
	$(CC) $(CFLAGS) -c perf_attr_print.c

filter_log:	filter_log.o
	$(CC) $(LFLAGS) -o filter_log filter_log.o

filter_log.o:	filter_log.c
	$(CC) $(CFLAGS) -c filter_log.c


replay_log:	replay_log.o \
		../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o replay_log replay_log.o ../lib/perf_helpers.o

replay_log.o:	replay_log.c
	$(CC) $(CFLAGS) -c replay_log.c


log_to_code:	log_to_code.o \
		perf_attr_print.o \
		../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o log_to_code log_to_code.o perf_attr_print.o ../lib/perf_helpers.o

log_to_code.o:	log_to_code.c perf_attr_print.h
	$(CC) $(CFLAGS) -c log_to_code.c



perf_fuzzer:	perf_fuzzer.o perf_attr_print.o \
		../lib/perf_helpers.o ../lib/instructions_testcode.o \
		fds.o interesting-numbers.o perf_event_open.o pids.o \
		random.o random-address.o random-length.o
	$(CC) $(LFLAGS) -o perf_fuzzer perf_fuzzer.o \
		perf_attr_print.o \
		../lib/perf_helpers.o ../lib/instructions_testcode.o \
		fds.o interesting-numbers.o perf_event_open.o pids.o \
		random.o random-address.o random-length.o

perf_fuzzer.o:	perf_fuzzer.c perf_attr_print.h
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

random-address.o:	random-address.c
	$(CC) $(CFLAGS) -c random-address.c

random-length.o:	random-length.c
	$(CC) $(CFLAGS) -c random-length.c

clean:	
	rm -f *~ *.o perf_fuzzer replay_log log_to_code filter_log

