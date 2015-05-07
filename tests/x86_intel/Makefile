CC = gcc
CFLAGS = -Wall -O2 -g -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	bts_aux \
	fixed_ctr0 \
	fixed_ctr1 \
	fixed_ctr2 \
	offcore_response \
	offcore_response_mask \
	pebs \
	pebs_addr \
	pebs_latency \
	pebs_registers \
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

$(LIB)/parse_record.o:	$(LIB)/parse_record.c
	cd $(LIB) && make

$(LIB)/perf_helpers.o:	$(LIB)/perf_helpers.c
	cd $(LIB) && make

$(LIB)/detect_cache.o:	$(LIB)/detect_cache.c
	cd $(LIB) && make

###

fixed_ctr0:	fixed_ctr0.o $(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o fixed_ctr0 fixed_ctr0.o \
			$(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o

fixed_ctr0.o:	fixed_ctr0.c
	$(CC) $(CFLAGS) -c fixed_ctr0.c
###

fixed_ctr1:	fixed_ctr1.o $(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o fixed_ctr1 fixed_ctr1.o \
			$(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o

fixed_ctr1.o:	fixed_ctr1.c
	$(CC) $(CFLAGS) -c fixed_ctr1.c

###

fixed_ctr2:	fixed_ctr2.o $(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o
	$(CC) $(LFLAGS) -o fixed_ctr2 fixed_ctr2.o \
			$(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o

fixed_ctr2.o:	fixed_ctr2.c
	$(CC) $(CFLAGS) -c fixed_ctr2.c

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

pebs_addr:	pebs_addr.o $(LIB)/test_utils.o \
			$(LIB)/matrix_multiply.o \
			$(LIB)/perf_helpers.o \
			$(LIB)/parse_record.o
	$(CC) $(LFLAGS) -o pebs_addr pebs_addr.o \
			$(LIB)/test_utils.o \
			$(LIB)/matrix_multiply.o \
			$(LIB)/perf_helpers.o \
			$(LIB)/parse_record.o

pebs_addr.o:	pebs_addr.c
	$(CC) $(CFLAGS) -c pebs_addr.c

####

bts_aux:	bts_aux.o $(LIB)/test_utils.o \
			$(LIB)/matrix_multiply.o \
			$(LIB)/perf_helpers.o \
			$(LIB)/parse_record.o
	$(CC) $(LFLAGS) -o bts_aux bts_aux.o \
			$(LIB)/test_utils.o \
			$(LIB)/matrix_multiply.o \
			$(LIB)/perf_helpers.o \
			$(LIB)/parse_record.o

bts_aux.o:	bts_aux.c
	$(CC) $(CFLAGS) -c bts_aux.c

###

pebs_latency:	pebs_latency.o $(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o \
			$(LIB)/parse_record.o
	$(CC) $(LFLAGS) -o pebs_latency pebs_latency.o \
			$(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o \
			$(LIB)/parse_record.o

pebs_latency.o:	pebs_latency.c
	$(CC) $(CFLAGS) -c pebs_latency.c

###

pebs_registers:	pebs_registers.o $(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o \
			$(LIB)/parse_record.o
	$(CC) $(LFLAGS) -o pebs_registers pebs_registers.o \
			$(LIB)/test_utils.o \
			$(LIB)/instructions_testcode.o \
			$(LIB)/perf_helpers.o \
			$(LIB)/parse_record.o

pebs_registers.o:	pebs_registers.c
	$(CC) $(CFLAGS) -c pebs_registers.c



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
	bts_aux \
	fixed_ctr0 \
	fixed_ctr1 \
	fixed_ctr2 \
	offcore_response \
	offcore_response_mask \
	pebs \
	pebs_latency \
	pebs_registers \
	uncore_events \
	uncore_cpu_cbox \
	uncore_max_cbox \
	uncore_group_cbox
