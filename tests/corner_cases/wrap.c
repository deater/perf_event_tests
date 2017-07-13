/* wrap.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* Testing wraparound on events */

/* This was broken from 3.11 until 4.9 before people noticed */

/* On Intel chips with "full-width counters" */
/* the max_period was set too high */

/* This is apparently easier to trigger on KNL (knight's landing) */
/* or SLM (silvermont) because their counters are smaller */

/* on broadwell it's set to 0000ffffffffffff, or 2^48 */
/* approx 300,000 * billion */
/* so with cycles will take roughly 28 hours to replicate? */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>


#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#define COUNTERS	2

int main(int argc, char** argv) {

	int i;
	int fd[COUNTERS],quiet;
	int result;
	int read_result;
	long long count[COUNTERS+1],last_count[COUNTERS+1];
	struct perf_event_attr pe[COUNTERS];

	char test_string[]="Make sure events always increase...";

	for(i=0;i<COUNTERS+1;i++) last_count[i]=0;

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing if wrapping works.\n");
	}

	memset(&pe[0],0,sizeof(struct perf_event_attr));

	pe[0].type=PERF_TYPE_HARDWARE;
   	pe[0].size=sizeof(struct perf_event_attr);
	pe[0].config=PERF_COUNT_HW_CPU_CYCLES;
	pe[0].disabled=1;
	pe[0].exclude_kernel=0;
	pe[0].read_format=PERF_FORMAT_GROUP;

	fd[0]=perf_event_open(&pe[0],0,-1,-1,0);
	if (fd[0]<0) {
		fprintf(stderr,"Error opening\n");
		test_fail(test_string);
		exit(1);
	}



	memset(&pe[1],0,sizeof(struct perf_event_attr));

	pe[1].type=PERF_TYPE_HARDWARE;
   	pe[1].size=sizeof(struct perf_event_attr);
	pe[1].config=PERF_COUNT_HW_INSTRUCTIONS;
	pe[1].disabled=0;
	pe[1].exclude_kernel=0;

	fd[1]=perf_event_open(&pe[1],0,-1,fd[0],0);
	if (fd[1]<0) {
		fprintf(stderr,"Error opening\n");
		test_fail(test_string);
		exit(1);
	}

#if 0
	memset(&pe[2],0,sizeof(struct perf_event_attr));

	pe[2].type=PERF_TYPE_RAW;
   	pe[2].size=sizeof(struct perf_event_attr);
	pe[2].config=0x5301b1;
	pe[2].disabled=0;
	pe[2].exclude_kernel=0;

	fd[2]=perf_event_open(&pe[1],0,-1,fd[0],0);
	if (fd[2]<0) {
		fprintf(stderr,"Error opening\n");
		test_fail(test_string);
		exit(1);
	}
#endif

	ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	while(1) {

		for(i=0;i<10000;i++) {
			result=instructions_million();
		}
		if (result==CODE_UNIMPLEMENTED) {
			test_skip(test_string);
		}

		read_result=read(fd[0],&count,sizeof(long long)*(COUNTERS+1));
		if (read_result!=sizeof(long long)*(COUNTERS+1)) {
		      printf("Unexpected read size\n");
		}

		if (count[1]<last_count[1]) {
			fprintf(stderr,"ERROR!  Count went backwards!\n");
			test_fail(test_string);
		}

		if (count[2]<last_count[2]) {
			fprintf(stderr,"ERROR!  Count went backwards!\n");
			test_fail(test_string);
		}
#if 0
		if (count[3]<last_count[3]) {
			fprintf(stderr,"ERROR!  Count went backwards!\n");
			test_fail(test_string);
		}
#endif

		last_count[0]=count[0];
		last_count[1]=count[1];
		last_count[2]=count[2];
#if 0
		last_count[3]=count[3];

		if (!quiet) {
			printf("\tCYC: %llx\tINS: %llx\tUOP %llx\n",
				count[1],count[2],count[3]);
      		}
#else

		if (!quiet) {
			printf("\tCYC: %llx\tINS: %llx\n",
				count[1],count[2]);
      		}
#endif

   	}

	close(fd[0]);
	close(fd[1]);
	close(fd[2]);

	test_pass(test_string);

	return 0;
}
