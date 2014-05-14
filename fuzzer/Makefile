CC = gcc

CFLAGS = -Wall -g -I../include -I./include -I./trinity_files/include
LFLAGS = 

CFLAGS_X32 = -Wall -mx32 -g -I../include -I./include -I./trinity_files/include
LFLAGS_X32 = -mx32 -static

####

all:	perf_fuzzer replay_log log_to_code filter_log active_events

x32:	perf_fuzzer.x32 replay_log.x32 log_to_code filter_log active_events


###

perf_attr_print.o:	perf_attr_print.c perf_attr_print.h
	$(CC) $(CFLAGS) -c perf_attr_print.c

perf_attr_print.x32.o:	perf_attr_print.c perf_attr_print.h
	$(CC) $(CFLAGS_X32) -o perf_attr_print.x32.o -c perf_attr_print.c

###

filter_log:	filter_log.o
	$(CC) $(LFLAGS) -o filter_log filter_log.o

filter_log.o:	filter_log.c
	$(CC) $(CFLAGS) -c filter_log.c

###

replay_log:	replay_log.o \
		../lib/perf_helpers.o perf_attr_print.o \
		trinity_files/trinity_lib.a
	$(CC) $(LFLAGS) -o replay_log replay_log.o ../lib/perf_helpers.o perf_attr_print.o trinity_files/trinity_lib.a

replay_log.o:	replay_log.c perf_attr_print.h
	$(CC) $(CFLAGS) -c replay_log.c

replay_log.x32:	replay_log.x32.o \
		../lib/perf_helpers.x32.o perf_attr_print.x32.o \
		trinity_files/trinity_lib.x32.a
	$(CC) $(LFLAGS_X32) -o replay_log.x32 replay_log.x32.o ../lib/perf_helpers.x32.o perf_attr_print.x32.o trinity_files/trinity_lib.x32.a

replay_log.x32.o:	replay_log.c perf_attr_print.h
	$(CC) $(CFLAGS_X32) -o replay_log.x32.o -c replay_log.c

###

active_events:	active_events.o \
		../lib/perf_helpers.o perf_attr_print.o
	$(CC) $(LFLAGS) -o active_events active_events.o ../lib/perf_helpers.o perf_attr_print.o

active_events.o:	active_events.c perf_attr_print.h
	$(CC) $(CFLAGS) -c active_events.c


###

log_to_code:	log_to_code.o \
		perf_attr_print.o \
		../lib/perf_helpers.o
	$(CC) $(LFLAGS) -o log_to_code log_to_code.o perf_attr_print.o ../lib/perf_helpers.o

log_to_code.o:	log_to_code.c perf_attr_print.h
	$(CC) $(CFLAGS) -c log_to_code.c

####

perf_fuzzer:	perf_fuzzer.o perf_attr_print.o \
		../lib/perf_helpers.o ../lib/instructions_testcode.o \
		trinity_files/trinity_lib.a
	$(CC) $(LFLAGS) -o perf_fuzzer perf_fuzzer.o \
		perf_attr_print.o \
		../lib/perf_helpers.o ../lib/instructions_testcode.o \
		trinity_files/trinity_lib.a

perf_fuzzer.o:	perf_fuzzer.c perf_attr_print.h version.h
	$(CC) $(CFLAGS) -c perf_fuzzer.c

####

perf_fuzzer.x32:	perf_fuzzer.x32.o perf_attr_print.x32.o \
		../lib/perf_helpers.x32.o ../lib/instructions_testcode.x32.o \
		trinity_files/trinity_lib.x32.a
	$(CC) $(LFLAGS_X32) -o perf_fuzzer.x32 perf_fuzzer.x32.o \
		perf_attr_print.x32.o \
		../lib/perf_helpers.x32.o ../lib/instructions_testcode.x32.o \
		trinity_files/trinity_lib.x32.a

perf_fuzzer.x32.o:	perf_fuzzer.c perf_attr_print.h version.h
	$(CC) $(CFLAGS_X32) -o perf_fuzzer.x32.o -c perf_fuzzer.c

####

trinity_files/trinity_lib.a:	
	cd trinity_files && make

trinity_files/trinity_lib.x32.a:	
	cd trinity_files && make x32

####

clean:	
	rm -f *~ *.o perf_fuzzer replay_log log_to_code filter_log active_events
	cd trinity_files && make clean

