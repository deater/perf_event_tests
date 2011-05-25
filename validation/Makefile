CC = gcc
CFLAGS = -Wall -O2
LFLAGS =

all:	branches instructions simple_overflow

###

branches:	branches.o test_utils.o branches_test.o helper.o
	$(CC) $(LFLAGS) -o branches branches.o test_utils.o branches_test.o helper.o

branches.o:	branches.c
	$(CC) $(CFLAGS) -c branches.c


branches_test.o:	branches_test.c
	$(CC) $(CFAGS) -c branches_test.c

helper.o:	helper.c
	$(CC) $(CFLAGS) -c helper.c


###

instructions:	instructions.o test_utils.o
	$(CC) $(LFLAGS) -o instructions instructions.o test_utils.o

instructions.o:	instructions.c
	$(CC) $(CFLAGS) -c instructions.c

###

simple_overflow:	simple_overflow.o test_utils.o
	$(CC) $(LFLAGS) -o simple_overflow simple_overflow.o test_utils.o

simple_overflow.o:	simple_overflow.c
	$(CC) $(CFLAGS) -c simple_overflow.c

###

test_utils.o:	test_utils.c
	$(CC) $(CFLAGS) -c test_utils.c

clean:	
	rm -f *~ *.o branches instructions simple_overflow


