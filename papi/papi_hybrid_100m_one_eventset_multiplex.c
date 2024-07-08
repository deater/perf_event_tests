/* This tests if hybrid events work */
/* testing with 100m assembly language instructions */

/* Test tries to have both P and E events in one eventset */
/* Tries hack of enabling multiplexing to see if it will work */
/* Can have both read, but results seem off.  Maybe it's scaling? */

/* by Vince Weaver, vincent.weaver@maine.edu        */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "papiStdEventDefs.h"
#include "papi.h"


//#include "test_utils.h"

//#include "matrix_multiply.h"
//#include "nops_testcode.h"

#include "instructions_testcode.h"

#define NUM_RUNS 100

void test_fail(char *string) {
	exit(-1);
}

void test_pass(char *string) {
	exit(0);
}

int main(int argc, char **argv) {

	int retval;
	const PAPI_hw_info_t *info;

	int eventset=PAPI_NULL;

	int i,quiet=0;
	long long counts[2],high=0,low=0;
	long long total_p=0,average_p=0;
	long long total_e=0,average_e=0;

	char test_string[]="Testing heterogeneous event...";

	retval = PAPI_library_init(PAPI_VER_CURRENT);
	if (retval != PAPI_VER_CURRENT) {
		if (!quiet) printf("PAPI_library_init %d\n", retval);
		test_fail(test_string);
	}

	if (!quiet) printf("\n");

	if ( (info=PAPI_get_hardware_info())==NULL) {
		if (!quiet) printf("Error! cannot obtain hardware info\n");
		test_fail(test_string);
	}


	retval = PAPI_multiplex_init(  );
        if ( retval != PAPI_OK ) {
                test_fail("PAPI multiplex init fail\n");
        }


	/* create eventset */

	retval=PAPI_create_eventset(&eventset);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error creating eventset!\n");
		test_fail(test_string);
	}

	retval=PAPI_add_named_event(eventset,"INST_RETIRED:ANY");
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error adding INST_RETIRED:ANY!\n");
		printf("%s\n",PAPI_strerror(retval));
		test_fail(test_string);
	}

	retval = PAPI_set_multiplex( eventset );
	if ( retval != PAPI_OK ) {
		if (!quiet) printf("Error enabling multiplex %s\n",
			PAPI_strerror(retval));
		test_fail("PAPI_set_multiplex");
        }

	retval=PAPI_add_named_event(eventset,"adl_grt::INST_RETIRED:ANY_P");
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error adding adl_grt::INST_RETIRED:ANY_P!\n");
		printf("%s\n",PAPI_strerror(retval));
		test_fail(test_string);
	}

	if (!quiet) {
		printf("Testing a 1 million instructions (%d times):\n",
			NUM_RUNS);
	}

	int start_result;
	int stop_result;

	for(i=0;i<NUM_RUNS;i++) {
//		printf("starting event\n");
		start_result=PAPI_start(eventset);
		if (start_result!=PAPI_OK) {
			if (!quiet) printf("Error starting event %s!\n",
				PAPI_strerror(start_result));
			test_fail("start");
		}

		instructions_million();

//		printf("stopping event\n");
		stop_result=PAPI_stop(eventset,counts);
		if (stop_result!=PAPI_OK) {
			if (!quiet) printf("Error stopping event %s!\n",
				PAPI_strerror(stop_result));
			test_fail("stop");
		}

		if (counts[0]>high) high=counts[0];
		if ((low==0) || (counts[0]<low)) low=counts[0];

		total_p+=counts[0];
		total_e+=counts[1];
	}

	average_p=total_p/NUM_RUNS;
	average_e=total_e/NUM_RUNS;

	if (!quiet) {
		printf("\tAverage should be low, as no user cycles when sleeping\n");
		printf("\tMeasured average p, e: %lld %lld\n",average_p,average_e);
	}

	if (!quiet) printf("\n");

	PAPI_shutdown();

	test_pass(test_string);

	return 0;
}
