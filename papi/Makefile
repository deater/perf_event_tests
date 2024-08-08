CC	= gcc
CC_R	= $(CC) -pthread
CFLAGS	= -g -O2 -Wall -I../include

include ../Makefile.papi.inc


all:	\
	check_papi_latency \
	core2_constraints \
	cpufreq_scaling \
	unregister \
	event_create_race \
	mpx_oflo \
	offcore_response \
	perfevent_sw_events \
	perfevent_generalized_events \
	papi_hybrid_sleep \
	papi_hybrid_100m_two_events \
	papi_hybrid_100m_one_eventset \
	papi_hybrid_100m_one_eventset_multiplex

###

papi_hybrid_sleep:	papi_hybrid_sleep.o
	$(CC) $(LFLAGS) -o papi_hybrid_sleep papi_hybrid_sleep.o \
		$(PAPI_LIB)

papi_hybrid_sleep.o:	papi_hybrid_sleep.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_hybrid_sleep.c


###

papi_hybrid_100m_two_events:	papi_hybrid_100m_two_events.o ../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o papi_hybrid_100m_two_events papi_hybrid_100m_two_events.o \
		../lib/instructions_testcode.o $(PAPI_LIB)

papi_hybrid_100m_two_events.o:	papi_hybrid_100m_two_events.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_hybrid_100m_two_events.c

###

papi_hybrid_100m_one_eventset:	papi_hybrid_100m_one_eventset.o ../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o papi_hybrid_100m_one_eventset papi_hybrid_100m_one_eventset.o \
		../lib/instructions_testcode.o $(PAPI_LIB)

papi_hybrid_100m_one_eventset.o:	papi_hybrid_100m_one_eventset.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_hybrid_100m_one_eventset.c

###

papi_hybrid_100m_one_eventset_multiplex:	papi_hybrid_100m_one_eventset_multiplex.o ../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o papi_hybrid_100m_one_eventset_multiplex papi_hybrid_100m_one_eventset_multiplex.o \
		../lib/instructions_testcode.o $(PAPI_LIB)

papi_hybrid_100m_one_eventset_multiplex.o:	papi_hybrid_100m_one_eventset_multiplex.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_hybrid_100m_one_eventset_multiplex.c


###

check_papi_latency:	check_papi_latency.o ../lib/test_utils.o ../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o check_papi_latency check_papi_latency.o \
		../lib/test_utils.o ../lib/instructions_testcode.o $(PAPI_LIB) -lrt

check_papi_latency.o:	check_papi_latency.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c check_papi_latency.c

####

core2_constraints:	core2_constraints.o ../lib/test_utils.o ../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o core2_constraints core2_constraints.o \
		../lib/test_utils.o ../lib/matrix_multiply.o \
		$(PAPI_LIB)


core2_constraints.o:	core2_constraints.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c core2_constraints.c


####

unregister:	unregister.o ../lib/test_utils.o 
	$(CC) $(LFLAGS) -o unregister unregister.o \
		../lib/test_utils.o \
		$(PAPI_LIB) -lpthread


unregister.o:	unregister.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c unregister.c

####

event_create_race:	event_create_race.o ../lib/test_utils.o 
	$(CC) $(LFLAGS) -o event_create_race event_create_race.o \
		../lib/test_utils.o \
		$(PAPI_LIB) -lpthread


event_create_race.o:	event_create_race.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c event_create_race.c

####

papi_helpers.o:	papi_helpers.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_helpers.c

####

mpx_oflo:	mpx_oflo.o ../lib/test_utils.o  ../lib/matrix_multiply.o papi_helpers.o
	$(CC) $(LFLAGS) -o mpx_oflo mpx_oflo.o \
		../lib/test_utils.o ../lib/matrix_multiply.o \
		$(PAPI_LIB) papi_helpers.o


mpx_oflo.o:	mpx_oflo.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c mpx_oflo.c


###

cpufreq_scaling:	cpufreq_scaling.o ../lib/test_utils.o \
		../lib/matrix_multiply.o ../lib/nops_testcode.o
	$(CC) $(LFLAGS) -o cpufreq_scaling cpufreq_scaling.o \
		../lib/test_utils.o \
		../lib/matrix_multiply.o ../lib/nops_testcode.o $(PAPI_LIB)

cpufreq_scaling.o:	cpufreq_scaling.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c cpufreq_scaling.c

###

offcore_response:	offcore_response.o ../lib/test_utils.o ../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o offcore_response offcore_response.o ../lib/test_utils.o ../lib/matrix_multiply.o $(PAPI_LIB)

offcore_response.o:	offcore_response.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c offcore_response.c	


###

perfevent_sw_events:	perfevent_sw_events.o ../lib/test_utils.o ../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o perfevent_sw_events perfevent_sw_events.o  ../lib/test_utils.o ../lib/matrix_multiply.o $(PAPI_LIB)

perfevent_sw_events.o:	perfevent_sw_events.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c perfevent_sw_events.c


###

perfevent_generalized_events:	perfevent_generalized_events.o ../lib/test_utils.o ../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o perfevent_generalized_events perfevent_generalized_events.o  ../lib/test_utils.o ../lib/matrix_multiply.o $(PAPI_LIB)

perfevent_generalized_events.o:	perfevent_generalized_events.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c perfevent_generalized_events.c


###

papi_profile:	papi_profile.o
	$(CC) $(LFLAGS) $(PAPI_LIB) -o papi_profile papi_profile.o

papi_profile.o:	papi_profile.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_profile.c


clean:
	rm -f *.o core *~ \
	check_mhz_value \
	check_papi_latency \
	core2_constraints \
	cpufreq_scaling \
	unregister \
	event_create_race \
	mpx_oflo \
	offcore_response \
	perfevent_sw_events \
	perfevent_generalized_events \
	papi_profile \
	papi_hybrid_sleep \
	papi_hybrid_100m_two_events \
	papi_hybrid_100m_one_eventset \
	papi_hybrid_100m_one_eventset_multiplex

install:	all
