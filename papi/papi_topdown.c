/* Test out PAPI topdown support */

/* at least originally just event-based on Zen5 */

/* by Vince Weaver, <vincent.weaver@maine.edu>        */

#define NUM_RUNS	1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include "papiStdEventDefs.h"
#include "papi.h"


int main(int argc, char **argv) {

	int retval;
	const PAPI_hw_info_t *info;

	int eventset=PAPI_NULL;

	int quiet=0;
	long long counts[7]={0,0,0,0,0,0,0};
	long long total_1=0;
	long long total_2=0;
	long long total_3=0;
	long long total_4=0;

	char *event_name1=NULL;
	char *event_name2=NULL;
	char *event_name3=NULL;
	char *event_name4=NULL;


	/* Init PAPI library */

	retval = PAPI_library_init(PAPI_VER_CURRENT);
	if (retval != PAPI_VER_CURRENT) {
		if (!quiet) printf("PAPI_library_init %d\n", retval);
		fprintf(stderr,"Error init\n");
		exit(-1);
	}

	if (!quiet) printf("\n");

	if ( (info=PAPI_get_hardware_info())==NULL) {
		if (!quiet) printf("Error! cannot obtain hardware info\n");
		fprintf(stderr,"Error getting hardware info\n");
		exit(-1);
	}
#if 0
	/* Raptor Lake */
	if ((info->vendor==PAPI_VENDOR_INTEL) && (info->cpuid_family==6) &&
            ((info->cpuid_model==183))) {

		instruction_event_p=strdup("adl_glc::INST_RETIRED:ANY");
		instruction_event_e=strdup("adl_grt::INST_RETIRED:ANY_P");
	}
	else {
		printf("Unknown vendor=%d, family=%d, model=%d\n",
			info->vendor, info->cpuid_family,
			info->cpuid_model);
		test_skip(test_string);
	}
#endif

	event_name1=strdup("CYCLES_NOT_IN_HALT:u=1:k=1");
	event_name2=strdup("DISPATCH_STALLS_1:FE_NO_OPS:u=1:k=1");
	event_name3=strdup("OPS_SOURCE_DISPATCHED_FROM_DECODER:DECODER:OPCACHE:k=1:u=1");
	event_name4=strdup("RETIRED_OPS:k=1:u=1");

	/* create eventset */

	retval=PAPI_create_eventset(&eventset);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error creating eventset!\n");
		fprintf(stderr,"Error creating eventset\n");
		exit(-1);
	}

	/* Try to open event1 */
	retval=PAPI_add_named_event(eventset,event_name1);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error adding %s\n",event_name1);
		printf("%s\n",PAPI_strerror(retval));
		fprintf(stderr,"Error adding event1\n");
		exit(-1);
	}

	/* Try to open event2 */
	retval=PAPI_add_named_event(eventset,event_name2);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error adding %s\n",event_name2);
		printf("%s\n",PAPI_strerror(retval));
		fprintf(stderr,"Error adding event2\n");
		exit(-1);
	}

	/* Try to open event3 */
	retval=PAPI_add_named_event(eventset,event_name3);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error adding %s\n",event_name3);
		printf("%s\n",PAPI_strerror(retval));
		fprintf(stderr,"Error adding event3\n");
		exit(-1);
	}

	/* Try to open event4 */
	retval=PAPI_add_named_event(eventset,event_name4);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error adding %s\n",event_name4);
		printf("%s\n",PAPI_strerror(retval));
		fprintf(stderr,"Error adding event4\n");
		exit(-1);
	}



	if (!quiet) {
		printf("Sleeping 1s\n");
	}

	int start_result;
	int stop_result;

//	for(i=0;i<NUM_RUNS;i++) {
		//printf("starting event\n");
		start_result=PAPI_start(eventset);
		if (start_result!=PAPI_OK) {
			if (!quiet) printf("Error starting event %s!\n",
				PAPI_strerror(start_result));
			fprintf(stderr,"Error starting event\n");
			exit(-1);

		}

		sleep(1);

		//printf("stopping event\n");
		stop_result=PAPI_stop(eventset,counts);
		if (stop_result!=PAPI_OK) {
			if (!quiet) printf("Error stopping event %s!\n",
				PAPI_strerror(stop_result));
			fprintf(stderr,"Error stopping event\n");
			exit(-1);
		}

		total_1=counts[0];
		total_2=counts[1];
		total_3=counts[2];
		total_4=counts[3];
//	}


	long long total_dispatch_slots;
	double frontend_bound;
	double bad_speculation;

	total_dispatch_slots=8*total_1;
	frontend_bound=(double)total_2/(double)total_dispatch_slots;
	bad_speculation=(total_3-total_4)/(double)total_dispatch_slots;

	if (!quiet) {
		printf("Total Dispatch Slots = %lld\n",total_dispatch_slots);
		printf("Frontend Bound = %.2lf%%\n",frontend_bound*100.0);
		printf("Bad Speculation = %.2lf%%\n",bad_speculation*100.0);
	}

	if (!quiet) printf("\n");

	PAPI_shutdown();

	return 0;
}
