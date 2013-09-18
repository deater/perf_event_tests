CC = gcc
CFLAGS = -Wall -O2 -I../../include
LFLAGS =
LIB = ../../lib

all:	\
	breakpoint_support \
	check_constraints \
	enable_on_exec \
	format_id_support \
	get_cache_info \
	inherit \
	inherit_stat \
	non-existent \
	offcore_response \
	offcore_response_mask \
	rdpmc_support \
	rdpmc_validation 

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

breakpoint_support:	breakpoint_support.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o 
	$(CC) $(LFLAGS) -o breakpoint_support breakpoint_support.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o 

breakpoint_support.o:	breakpoint_support.c
	$(CC) $(CFLAGS) -c breakpoint_support.c

###

check_constraints:	check_constraints.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o check_constraints check_constraints.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/matrix_multiply.o

check_constraints.o:	check_constraints.c
	$(CC) $(CFLAGS) -c check_constraints.c

###

get_cache_info:	get_cache_info.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/detect_cache.o
	$(CC) $(LFLAGS) -o get_cache_info get_cache_info.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/detect_cache.o

get_cache_info.o:	get_cache_info.c
	$(CC) $(CFLAGS) -c get_cache_info.c

###

inherit:	inherit.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o 
	$(CC) $(LFLAGS) -o inherit inherit.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o -lpthread

inherit.o:	inherit.c
	$(CC) $(CFLAGS) -c inherit.c

###

inherit_stat:	inherit_stat.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o inherit_stat inherit_stat.o \
				$(LIB)/test_utils.o \
				$(LIB)/instructions_testcode.o \
				$(LIB)/perf_helpers.o -lpthread

inherit_stat.o:	inherit_stat.c
	$(CC) $(CFLAGS) -c inherit_stat.c


###

non-existent:	non-existent.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o non-existent non-existent.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/matrix_multiply.o

non-existent.o:	non-existent.c
	$(CC) $(CFLAGS) -c non-existent.c

###

enable_on_exec:	enable_on_exec.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o enable_on_exec enable_on_exec.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

enable_on_exec.o:	enable_on_exec.c
	$(CC) $(CFLAGS) -c enable_on_exec.c


###                                                                            

format_id_support:	format_id_support.o \
	$(LIB)/perf_helpers.o \
	$(LIB)/test_utils.o \
	$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o format_id_support format_id_support.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/test_utils.o \
				$(LIB)/matrix_multiply.o

format_id_support.o:	format_id_support.c
	$(CC) $(CFLAGS) -c format_id_support.c

###

rdpmc_support:	rdpmc_support.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/matrix_multiply.o
	$(CC) $(LFLAGS) -o rdpmc_support rdpmc_support.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/matrix_multiply.o

rdpmc_support.o:	rdpmc_support.c
	$(CC) $(CFLAGS) -c rdpmc_support.c

###

rdpmc_validation:	rdpmc_validation.o $(LIB)/test_utils.o \
					$(LIB)/perf_helpers.o \
					$(LIB)/instructions_testcode.o
	$(CC) $(LFLAGS) -o rdpmc_validation rdpmc_validation.o \
				$(LIB)/test_utils.o \
				$(LIB)/perf_helpers.o \
				$(LIB)/instructions_testcode.o

rdpmc_validation.o:	rdpmc_validation.c
	$(CC) $(CFLAGS) -c rdpmc_validation.c


clean:	
	rm -f *~ *.o \
	breakpoint_support \
	check_constraints \
	enable_on_exec \
	format_id_support \
	get_cache_info \
	inherit \
	inherit_stat \
	non-existent \
	offcore_response \
	offcore_response_mask \
	rdpmc_support \
	rdpmc_validation 
