/* This tests if hybrid events work */
/* testing with 100m assembly language instructions */

/* Tries measuring two eventsets at once, one E, one P */
/* PAPI currently does not support this */

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

	int eventset[2]={PAPI_NULL,PAPI_NULL};

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

	/* create p-core eventset */

	retval=PAPI_create_eventset(&eventset[0]);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error creating eventset!\n");
		test_fail(test_string);
	}

	retval=PAPI_add_named_event(eventset[0],"INST_RETIRED:ANY");
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error adding INST_RETIRED:ANY!\n");
		printf("%s\n",PAPI_strerror(retval));
		test_fail(test_string);
	}

	/* create E-core eventset */

	retval=PAPI_create_eventset(&eventset[1]);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error creating eventset!\n");
		test_fail(test_string);
	}

	retval=PAPI_add_named_event(eventset[1],"adl_grt::INST_RETIRED:ANY_P");
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error adding adl_grt::INST_RETIRED:ANY_P!\n");
		printf("%s\n",PAPI_strerror(retval));
		test_fail(test_string);
	}

	if (!quiet) {
		printf("Testing a 1 million instructions (%d times):\n",
			NUM_RUNS);
	}

	int start1_result,start2_result;
	int stop1_result,stop2_result;

	for(i=0;i<NUM_RUNS;i++) {
		printf("starting event1\n");
		start1_result=PAPI_start(eventset[0]);
		if (start1_result!=PAPI_OK) {
			if (!quiet) printf("Error starting event1 %s!\n",
				PAPI_strerror(start1_result));
			test_fail("start1");
		}
		printf("starting event2\n");
		start2_result=PAPI_start(eventset[1]);
		if (start2_result!=PAPI_OK) {
			if (!quiet) printf("Error starting event2 %s!\n",
				PAPI_strerror(start2_result));
			test_fail("start2");
		}

		instructions_million();

		printf("stopping event1\n");
		stop1_result=PAPI_stop(eventset[0],&counts[0]);
		if (stop1_result!=PAPI_OK) {
			if (!quiet) printf("Error stopping event1 %s!\n",
				PAPI_strerror(stop1_result));
			test_fail("stop1");
		}

		printf("stopping event2\n");
		stop2_result=PAPI_stop(eventset[1],&counts[1]);

		if (stop2_result!=PAPI_OK) {
			if (!quiet) printf("Error stopping event2 %s!\n",
				PAPI_strerror(stop2_result));
			test_fail("stop2");
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
