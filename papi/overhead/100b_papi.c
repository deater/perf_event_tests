/* This program measures the relative overhead of PAPI	*/
/* vs raw perf_event					*/

/* by Vince Weaver, vincent.weaver _at_ maine.edu	*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "instructions_testcode.h"

#define NUM_RUNS 100000



long long results[NUM_RUNS];

int main(int argc, char **argv) {

	int retval;

	retval = PAPI_library_init(PAPI_VER_CURRENT);
	if (retval != PAPI_VER_CURRENT) {
		fprintf(stderr,"Error! PAPI_library_init %d\n", retval);
	}

	retval = PAPI_query_event(PAPI_TOT_INS);
	if (retval != PAPI_OK) {
		fprintf(stderr,"PAPI_TOT_INS not supported\n");
		exit(1);
	}

	int i;
	int events[1],result;
	long long counts[1];

	long long total=0,average,max=0,min=0x7ffffffffffffffULL;

	events[0]=PAPI_TOT_INS;

	PAPI_start_counters(events,1);

	for(i=0;i<NUM_RUNS;i++) {


		result=instructions_million();

		PAPI_read_counters(counts,1);

		results[i]=counts[0];

 	}

	PAPI_stop_counters(counts,1);


	PAPI_shutdown();

	for(i=0;i<NUM_RUNS;i++) {
		total+=results[i];
		if (results[i]>max) max=results[i];
		if (results[i]<min) min=results[i];
	}

	average=total/NUM_RUNS;
	printf("Average=%lld max=%lld min=%lld\n",average,max,min);

	(void) result;

	return 0;
}
