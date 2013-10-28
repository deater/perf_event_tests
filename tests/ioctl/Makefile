CC = gcc
CFLAGS = -Wall -O2 -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	ioctl_flag_group \
	ioctl_id \
	ioctl_period

###

$(LIB)/test_utils.o:	$(LIB)/test_utils.c
	cd $(LIB) && make

$(LIB)/branches_testcode.o:	$(LIB)/branches_testcode.c
	cd $(LIB) && make

$(LIB)/matrix_multiply.o:	$(LIB)/matrix_multiply.c
	cd $(LIB) && make

$(LIB)/perf_helpers.o:	$(LIB)/perf_helpers.c
	cd $(LIB) && make

$(LIB)/detect_cache.o:	$(LIB)/detect_cache.c
	cd $(LIB) && make

###

ioctl_flag_group:	ioctl_flag_group.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o ioctl_flag_group ioctl_flag_group.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o \
				$(LIB)/perf_helpers.o -lpthread

ioctl_flag_group.o:	ioctl_flag_group.c
	$(CC) $(CFLAGS) -c ioctl_flag_group.c

###

ioctl_id:	ioctl_id.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o ioctl_id ioctl_id.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o \
				$(LIB)/perf_helpers.o -lpthread

ioctl_id.o:	ioctl_id.c
	$(CC) $(CFLAGS) -c ioctl_id.c




###

ioctl_period:	ioctl_period.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o ioctl_period ioctl_period.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o \
				$(LIB)/perf_helpers.o -lpthread

ioctl_period.o:	ioctl_period.c
	$(CC) $(CFLAGS) -c ioctl_period.c



####

clean:	
	rm -f *~ *.o \
	ioctl_flag_group \
	ioctl_id \
	ioctl_period
