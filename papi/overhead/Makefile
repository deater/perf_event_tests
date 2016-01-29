CC = gcc
CFLAGS = -Wall -O2 -I../../include
LFLAGS =

all:	papi_tot_ins parse_output

papi_tot_ins:	papi_tot_ins.o
	$(CC) $(LFLAGS) -o papi_tot_ins papi_tot_ins.o ../../lib/instructions_testcode.o

papi_tot_ins.o:	papi_tot_ins.c
	$(CC) $(CFLAGS) -c papi_tot_ins.c


parse_output:	parse_output.o
	$(CC) $(LFLAGS) -o parse_output parse_output.o

parse_output.o:	parse_output.c
	$(CC) $(LFLAGS) -c parse_output.c




clean:	
	rm -f *.o *~ \
		papi_tot_ins parse_output
