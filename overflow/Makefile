CC = gcc
CFLAGS = -Wall -O2 -I../include
LFLAGS =

all:	\
	breakpoint_overflow \
	overflow_requires_wakeup \
	simple_overflow_leader \
	simple_overflow_sibling \
	simultaneous_group_overflow \
	simul_oneshot_group_overflow \
	simultaneous_overflow  \
	single_shot_overflow \
	single_shot_w_enable \
	wakeup_events_overflow


###

../lib/test_utils.o:	../lib/test_utils.c
	cd ../lib && make

../lib/branches_testcode.o:	../lib/branches_testcode.c
	cd ../lib && make

../lib/matrix_multiply.o:	../lib/matrix_multiply.c
	cd ../lib && make

../lib/perf_helpers.o:	../lib/perf_helpers.c
	cd ../lib && make

../lib/detect_cache.o:	../lib/detect_cache.c
	cd ../lib && make

###

breakpoint_overflow:	breakpoint_overflow.o ../lib/test_utils.o \
					../lib/perf_helpers.o 
	$(CC) $(LFLAGS) -o breakpoint_overflow breakpoint_overflow.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o 

breakpoint_overflow.o:	breakpoint_overflow.c
	$(CC) $(CFLAGS) -c breakpoint_overflow.c


###

simultaneous_group_overflow:	simultaneous_group_overflow.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o simultaneous_group_overflow simultaneous_group_overflow.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/matrix_multiply.o

simultaneous_group_overflow.o:	simultaneous_group_overflow.c
	$(CC) $(CFLAGS) -c simultaneous_group_overflow.c


###

simul_oneshot_group_overflow:	simul_oneshot_group_overflow.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o simul_oneshot_group_overflow simul_oneshot_group_overflow.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/matrix_multiply.o

simul_oneshot_group_overflow.o:	simul_oneshot_group_overflow.c
	$(CC) $(CFLAGS) -c simul_oneshot_group_overflow.c


###



simultaneous_overflow:	simultaneous_overflow.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o simultaneous_overflow simultaneous_overflow.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/matrix_multiply.o

simultaneous_overflow.o:	simultaneous_overflow.c
	$(CC) $(CFLAGS) -c simultaneous_overflow.c


###

overflow_requires_wakeup:	overflow_requires_wakeup.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o overflow_requires_wakeup overflow_requires_wakeup.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/instructions_testcode.o

overflow_requires_wakeup.o:	overflow_requires_wakeup.c
	$(CC) $(CFLAGS) -c overflow_requires_wakeup.c


###

simple_overflow_leader:	simple_overflow_leader.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o simple_overflow_leader simple_overflow_leader.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/instructions_testcode.o

simple_overflow_leader.o:	simple_overflow_leader.c
	$(CC) $(CFLAGS) -c simple_overflow_leader.c




###

simple_overflow_sibling:	simple_overflow_sibling.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o simple_overflow_sibling simple_overflow_sibling.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/instructions_testcode.o

simple_overflow_sibling.o:	simple_overflow_sibling.c
	$(CC) $(CFLAGS) -c simple_overflow_sibling.c


###

single_shot_overflow:	single_shot_overflow.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o single_shot_overflow single_shot_overflow.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/instructions_testcode.o

single_shot_overflow.o:	single_shot_overflow.c
	$(CC) $(CFLAGS) -c single_shot_overflow.c

###

single_shot_w_enable:	single_shot_w_enable.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o single_shot_w_enable single_shot_w_enable.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/instructions_testcode.o

single_shot_w_enable.o:	single_shot_w_enable.c
	$(CC) $(CFLAGS) -c single_shot_w_enable.c

###

wakeup_events_overflow:	wakeup_events_overflow.o ../lib/test_utils.o \
					../lib/perf_helpers.o \
					../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o wakeup_events_overflow wakeup_events_overflow.o \
				../lib/test_utils.o \
				../lib/perf_helpers.o \
				../lib/instructions_testcode.o

wakeup_events_overflow.o:	wakeup_events_overflow.c
	$(CC) $(CFLAGS) -c wakeup_events_overflow.c


clean:	
	rm -f *~ *.o \
	breakpoint_overflow \
	overflow_requires_wakeup \
	simple_overflow_leader \
	simple_overflow_sibling \
	simultaneous_group_overflow \
	simul_oneshot_group_overflow \
	simultaneous_overflow \
	single_shot_overflow \
	single_shot_w_enable \
	wakeup_events_overflow

