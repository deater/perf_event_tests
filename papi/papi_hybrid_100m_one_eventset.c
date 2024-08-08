/* This tests if hybrid events work */
/* testing with 100m assembly language instructions */

/* Test tries to have both P and E events in one eventset */

/* by Vince Weaver, vincent.weaver@maine.edu        */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "instructions_testcode.h"

#define NUM_RUNS 100

void test_fail(char *string) {
	printf("Test failed...\n");
	exit(-1);
}

void test_pass(char *string) {
	exit(0);
}

void test_skip(char *string) {
	printf("Test skipped...\n");
	exit(1);
}

int main(int argc, char **argv) {

	int retval;
	const PAPI_hw_info_t *info;

	int eventset=PAPI_NULL;

	int i,quiet=0;
	long long counts[2]={0,0},high=0,low=0;
	long long total_p=0,average_p=0;
	long long total_e=0,average_e=0;

	char *instruction_event_p=NULL;
	char *instruction_event_e=NULL;


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

	/* Raptor Lake */
	if ((info->vendor==PAPI_VENDOR_INTEL) && (info->cpuid_family==6) &&
            ((info->cpuid_model==183))) {

		instruction_event_p=strdup("adl_glc::INST_RETIRED:ANY");
		instruction_event_e=strdup("adl_grt::INST_RETIRED:ANY_P");
	}

	/* Orangepi Arm Cortex A53/A72 */
	else if ((info->vendor==PAPI_VENDOR_ARM_ARM) && (info->cpuid_family==8) &&
            ((info->cpuid_model==3331))) {
		instruction_event_p=strdup("arm_ac72::INST_RETIRED");
		instruction_event_e=strdup("arm_ac53::INST_RETIRED");

	}
	else {
		printf("Unknown vendor=%d, family=%d, model=%d\n",
			info->vendor, info->cpuid_family,
			info->cpuid_model);
		test_skip(test_string);
	}

	/* create eventset */

	retval=PAPI_create_eventset(&eventset);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error creating eventset!\n");
		test_fail(test_string);
	}

	/* Try to open performance/BIG event */
	retval=PAPI_add_named_event(eventset,instruction_event_p);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error adding %s\n",instruction_event_p);
		printf("%s\n",PAPI_strerror(retval));
		test_fail(test_string);
	}

	/* Try to open performance/little event */
	retval=PAPI_add_named_event(eventset,instruction_event_e);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error adding %s\n",instruction_event_e);
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
		//printf("starting event\n");
		start_result=PAPI_start(eventset);
		if (start_result!=PAPI_OK) {
			if (!quiet) printf("Error starting event %s!\n",
				PAPI_strerror(start_result));
			test_fail("start");
		}

		instructions_million();

		//printf("stopping event\n");
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
