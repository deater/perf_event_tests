CC = gcc
CFLAGS = -Wall -O2 -I../../include
LFLAGS =

all:	100b_none 100b_papi 100b_perf 100b_perf_rdpmc parse_output

100b_none:	100b_none.o
	$(CC) $(LFLAGS) -o 100b_none 100b_none.o ../../lib/instructions_testcode.o

100b_none.o:	100b_none.c
	$(CC) $(CFLAGS) -c 100b_none.c


100b_perf:	100b_perf.o
	$(CC) $(LFLAGS) -o 100b_perf 100b_perf.o \
			../../lib/instructions_testcode.o \
			../../lib/perf_helpers.o

100b_perf.o:	100b_perf.c
	$(CC) $(CFLAGS) -c 100b_perf.c


100b_perf_rdpmc:	100b_perf_rdpmc.o
	$(CC) $(LFLAGS) -o 100b_perf_rdpmc 100b_perf_rdpmc.o \
			../../lib/instructions_testcode.o \
			../../lib/perf_helpers.o

100b_perf_rdpmc.o:	100b_perf_rdpmc.c
	$(CC) $(CFLAGS) -c 100b_perf_rdpmc.c





100b_papi:	100b_papi.o
	$(CC) $(LFLAGS) -o 100b_papi 100b_papi.o ../../lib/instructions_testcode.o ~/research/papi.git/src/libpapi.a

100b_papi.o:	100b_papi.c
	$(CC) $(CFLAGS) -c 100b_papi.c


parse_output:	parse_output.o
	$(CC) $(LFLAGS) -o parse_output parse_output.o

parse_output.o:	parse_output.c
	$(CC) $(LFLAGS) -c parse_output.c


clean:	
	rm -f *.o *~ \
		100b_none 100b_perf 100b_perf_rdpmc 100b_papi parse_output
