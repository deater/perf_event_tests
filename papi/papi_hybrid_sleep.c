/* This tests if heterogeneous events work */

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


#define SLEEP_RUNS 3

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
		printf("Testing a sleep of 1 second (%d times):\n",
			SLEEP_RUNS);
	}

	for(i=0;i<SLEEP_RUNS;i++) {
		PAPI_start(eventset[0]);
		PAPI_start(eventset[1]);
		sleep(1);
		PAPI_stop(eventset[0],&counts[0]);
		PAPI_stop(eventset[1],&counts[1]);

		if (counts[0]>high) high=counts[0];
		if ((low==0) || (counts[0]<low)) low=counts[0];

		total_p+=counts[0];
		total_e+=counts[1];
	}

	average_p=total_p/SLEEP_RUNS;
	average_e=total_e/SLEEP_RUNS;

	if (!quiet) {
		printf("\tAverage should be low, as no user cycles when sleeping\n");
		printf("\tMeasured average p, e: %lld %lld\n",average_p,average_e);
	}

	if (!quiet) printf("\n");

	PAPI_shutdown();

	test_pass(test_string);

	return 0;
}
