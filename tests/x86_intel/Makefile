CC = gcc
CFLAGS = -Wall -O2 -g -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	offcore_response \
	offcore_response_mask \
	pebs \
	uncore_events \
	uncore_cpu_cbox \
	uncore_max_cbox \
	uncore_group_cbox

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

offcore_response:	offcore_response.o $(LIB)/test_utils.o \
			$(LIB)/matrix_multiply.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o offcore_response offcore_response.o \
			$(LIB)/test_utils.o \
			$(LIB)/matrix_multiply.o \
			$(LIB)/perf_helpers.o

offcore_response.o:	offcore_response.c
	$(CC) $(CFLAGS) -c offcore_response.c

###

pebs:	pebs.o $(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o pebs pebs.o \
			$(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o

pebs.o:	pebs.c
	$(CC) $(CFLAGS) -c pebs.c



###

offcore_response_mask:	offcore_response_mask.o $(LIB)/test_utils.o \
			$(LIB)/matrix_multiply.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o offcore_response_mask offcore_response_mask.o \
			$(LIB)/test_utils.o \
			$(LIB)/matrix_multiply.o \
			$(LIB)/perf_helpers.o

offcore_response_mask.o:	offcore_response_mask.c
	$(CC) $(CFLAGS) -c offcore_response_mask.c

###

uncore_events:	uncore_events.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o uncore_events uncore_events.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/matrix_multiply.o

uncore_events.o:	uncore_events.c
	$(CC) $(CFLAGS) -c uncore_events.c


###

uncore_max_cbox:	uncore_max_cbox.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o uncore_max_cbox uncore_max_cbox.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/matrix_multiply.o

uncore_max_cbox.o:	uncore_max_cbox.c
	$(CC) $(CFLAGS) -c uncore_max_cbox.c


###

uncore_group_cbox:	uncore_group_cbox.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o uncore_group_cbox uncore_group_cbox.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/matrix_multiply.o

uncore_group_cbox.o:	uncore_group_cbox.c
	$(CC) $(CFLAGS) -c uncore_group_cbox.c

###

uncore_cpu_cbox:	uncore_cpu_cbox.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o uncore_cpu_cbox uncore_cpu_cbox.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/matrix_multiply.o

uncore_cpu_cbox.o:	uncore_cpu_cbox.c
	$(CC) $(CFLAGS) -c uncore_cpu_cbox.c


###


clean:	
	rm -f *~ *.o \
	offcore_response \
	offcore_response_mask \
	pebs \
	uncore_events \
	uncore_cpu_cbox \
	uncore_max_cbox \
	uncore_group_cbox
