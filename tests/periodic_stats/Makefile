CC = gcc
CFLAGS = -Wall -O2 -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	print_record_sample 

###

print_record_sample:	print_record_sample.o
	$(CC) $(LFLAGS) -o print_record_sample \
				print_record_sample.o 

print_record_sample.o:	print_record_sample.c
	$(CC) $(CFLAGS) -c print_record_sample.c

###

clean:	
	rm -f *~ *.o \
	print_record_sample 
