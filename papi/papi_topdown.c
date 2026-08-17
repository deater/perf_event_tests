/* FIXME: raw event for missing one */

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


/******************************************************/
/******************************************************/
/**************** Level 1 *** *************************/
/******************************************************/
/******************************************************/

static int zen5_level1(void) {

	int retval;
	int eventset=PAPI_NULL;

	int quiet=0;
	long long counts[7]={0,0,0,0,0,0,0};
	long long total_1=0;
	long long total_2=0;
	long long total_3=0;
	long long total_4=0;
	long long total_5=0;
	long long total_6=0;

	char *event_name1=NULL;
	char *event_name2=NULL;
	char *event_name3=NULL;
	char *event_name4=NULL;
	char *event_name5=NULL;
	char *event_name6=NULL;

	event_name1=strdup("CYCLES_NOT_IN_HALT:u=1:k=1");
	event_name2=strdup("DISPATCH_STALLS_1:FE_NO_OPS:u=1:k=1");
	event_name3=strdup("OPS_SOURCE_DISPATCHED_FROM_DECODER:DECODER:OPCACHE:k=1:u=1");
	event_name4=strdup("RETIRED_OPS:k=1:u=1");
	event_name5=strdup("DISPATCH_STALLS_1:BE_STALLS:u=1:k=1");
	event_name6=strdup("DISPATCH_STALLS_1:SMT_CONTENTION:u=1:k=1");

	printf("Level 1\n");

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

	/* Try to open event5 */
	retval=PAPI_add_named_event(eventset,event_name5);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error adding %s\n",event_name5);
		printf("%s\n",PAPI_strerror(retval));
		fprintf(stderr,"Error adding event5\n");
		exit(-1);
	}

	/* Try to open event6 */
	retval=PAPI_add_named_event(eventset,event_name6);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error adding %s\n",event_name6);
		printf("%s\n",PAPI_strerror(retval));
		fprintf(stderr,"Error adding event6\n");
//		exit(-1);
	}




	if (!quiet) {
		printf("\tSleeping 1s\n");
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
		total_5=counts[4];
		total_6=counts[5];
//	}


	long long total_dispatch_slots;
	double frontend_bound;
	double bad_speculation;
	double backend_bound;
	double smt_contention;
	double retiring;

	total_dispatch_slots=8*total_1;
	frontend_bound=(double)total_2/(double)total_dispatch_slots;
	bad_speculation=(total_3-total_4)/(double)total_dispatch_slots;
	backend_bound=(total_5)/(double)total_dispatch_slots;
	smt_contention=(total_6)/(double)total_dispatch_slots;
	retiring=(total_4)/(double)total_dispatch_slots;

	if (!quiet) {
		printf("\tTotal Dispatch Slots = %lld\n",total_dispatch_slots);
		printf("\tFrontend Bound = %.2lf%%\n",frontend_bound*100.0);
		printf("\tBad Speculation = %.2lf%%\n",bad_speculation*100.0);
		printf("\tBackend Bound = %.2lf%%\n",backend_bound*100.0);
		printf("\tSMT Contention = %.2lf%%\n",smt_contention*100.0);
		printf("\tRetiring = %.2lf%%\n",retiring*100.0);
	}

	if (!quiet) printf("\n");

	PAPI_destroy_eventset(&eventset);

	return 0;

}

/******************************************************/
/******************************************************/
/**************** L2 Frontend *************************/
/******************************************************/
/******************************************************/

static int zen5_level2_frontend(void) {

	int retval;
	int eventset=PAPI_NULL;

	int quiet=0;
	long long counts[3]={0,0,0};
	long long total_1=0;
	long long total_2=0;
	long long total_3=0;

	char *event_name1=NULL;
	char *event_name2=NULL;
	char *event_name3=NULL;

	event_name1=strdup("CYCLES_NOT_IN_HALT:u=1:k=1");
	event_name2=strdup("DISPATCH_STALLS_1:FE_NO_OPS:u=1:k=1:c=8");
	event_name3=strdup("DISPATCH_STALLS_1:FE_NO_OPS:u=1:k=1");

	printf("Level2 Frontend Bound\n");

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

	if (!quiet) {
		printf("\tSleeping 1s\n");
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

//	}


	long long total_dispatch_slots;
	double frontend_bound_latency;
	double frontend_bound_bw;

	total_dispatch_slots=8*total_1;
	frontend_bound_latency=(double)8.0*(double)total_2/
		(double)total_dispatch_slots;
	frontend_bound_bw=(total_3-8*total_2)/(double)total_dispatch_slots;

	if (!quiet) {
		printf("\tTotal Dispatch Slots = %lld\n",total_dispatch_slots);
		printf("\tFrontend Bound Latency = %.2lf%%\n",frontend_bound_latency*100.0);
		printf("\tFrontend Bound Bandwidth = %.2lf%%\n",frontend_bound_bw*100.0);
	}

	if (!quiet) printf("\n");

	PAPI_destroy_eventset(&eventset);

	return 0;

}


/******************************************************/
/******************************************************/
/**************** Level 2 Bad Speculation  ************/
/******************************************************/
/******************************************************/

static int zen5_level2_bad_speculation(void) {

	int retval;
	int eventset=PAPI_NULL;

	int quiet=0;
	long long counts[5]={0,0,0,0,0};
	long long total_1=0;
	long long total_2=0;
	long long total_3=0;
	long long total_4=0;
	long long total_5=0;

	char *event_name1=NULL;
	char *event_name2=NULL;
	char *event_name3=NULL;
	char *event_name4=NULL;
	char *event_name5=NULL;

	event_name1=strdup("CYCLES_NOT_IN_HALT:u=1:k=1");
	event_name2=strdup("OPS_SOURCE_DISPATCHED_FROM_DECODER:DECODER:OPCACHE:k=1:u=1");
	event_name3=strdup("RETIRED_OPS:k=1:u=1");
	event_name4=strdup("RETIRED_BRANCH_INSTRUCTIONS_MISPREDICTED:u=1:k=1");
	event_name5=strdup("BP_REDIRECTS:RESYNC:u=1:k=1");

	printf("Level 2 Bad Speculation\n");

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

	/* Try to open event5 */
	retval=PAPI_add_named_event(eventset,event_name5);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error adding %s\n",event_name5);
		printf("%s\n",PAPI_strerror(retval));
		fprintf(stderr,"Error adding event5\n");
		exit(-1);
	}

	if (!quiet) {
		printf("\tSleeping 1s\n");
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
		total_5=counts[4];
//	}


	long long total_dispatch_slots;
	double bad_speculation;
	double bad_speculation_mispredicts;
	double bad_speculation_pipeline_restarts;

	total_dispatch_slots=8*total_1;
	bad_speculation=(total_2-total_3)/(double)total_dispatch_slots;
	bad_speculation_mispredicts=(double)(bad_speculation*total_4)/
			(double)(total_4+total_5);
	bad_speculation_pipeline_restarts=(double)(bad_speculation*total_5)/
			(double)(total_4+total_5);

	if (!quiet) {
		printf("\tTotal Dispatch Slots = %lld\n",total_dispatch_slots);
		printf("\tBad Speculation = %.2lf%%\n",bad_speculation*100.0);
		printf("\tBad Speculation Mispredicts = %.2lf%%\n",bad_speculation_mispredicts*100.0);
		printf("\tBad Speculation Pipeline Restarts = %.2lf%%\n",bad_speculation_pipeline_restarts*100.0);
	}

	if (!quiet) printf("\n");

	PAPI_destroy_eventset(&eventset);

	return 0;

}



int main(int argc, char **argv) {

	int quiet=0;
	int retval;
	const PAPI_hw_info_t *info;

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

	zen5_level1();
	zen5_level2_frontend();
	zen5_level2_bad_speculation();

	PAPI_shutdown();

	return 0;
}
