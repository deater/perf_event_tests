/* check_papi_multiplexing.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* The PAPI multiplexing code partitions events that don't fit in */
/*  the HW counters.  This requires failure on open, as checked   */
/*  by the existing check_schedulability test.  The failure mode  */
/*  is slightly different though so check for this too.           */


#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include <signal.h>

#include <sys/mman.h>

#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/prctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "matrix_multiply.h"

#define NUM_EVENTS 14

int main(int argc, char** argv) {

	int ret,quiet,i,result;

	struct perf_event_attr pe;

	char test_string[]="Testing if schedulability checked with pinned=0...";

	int fd[NUM_EVENTS];

	long long events[NUM_EVENTS]={
		PERF_COUNT_HW_CPU_CYCLES,
		PERF_COUNT_HW_INSTRUCTIONS,
		PERF_COUNT_HW_CACHE_REFERENCES,
		PERF_COUNT_HW_CACHE_MISSES,
		PERF_COUNT_HW_BRANCH_MISSES,
		PERF_COUNT_HW_BRANCH_INSTRUCTIONS,
		PERF_COUNT_HW_BUS_CYCLES,
		PERF_COUNT_HW_CPU_CYCLES,
		PERF_COUNT_HW_INSTRUCTIONS,
		PERF_COUNT_HW_CACHE_REFERENCES,
		PERF_COUNT_HW_CACHE_MISSES,
		PERF_COUNT_HW_BRANCH_MISSES,
		PERF_COUNT_HW_BRANCH_INSTRUCTIONS,
		PERF_COUNT_HW_BUS_CYCLES,
	};

	quiet=test_quiet();

	if (!quiet) {
		printf("Before 2.6.33 events weren't checked to see if they could\n");
		printf("  run together until read time, rather than at open time.\n");
		printf("This broke PAPI multiplexing which depends on knowing at open.\n");
		printf("  in order to set up partitioning.  This is a different error\n");
		printf("  case than check_schedulability as the group leader isn't pinned.\n");
	}

	fd[0]=-1;

	for(i=0;i<NUM_EVENTS;i++) {

		memset(&pe,0,sizeof(struct perf_event_attr));
		pe.type=PERF_TYPE_HARDWARE;
		pe.size=sizeof(struct perf_event_attr);
		pe.config=events[i];
		if (i==0) {
			pe.disabled=1;
		}
		pe.exclude_kernel=1;
		pe.exclude_hv=1;

		arch_adjust_domain(&pe,quiet);

		fd[i]=perf_event_open(&pe,0,-1,fd[0],0);
		if (fd[i]<0) {
			if (!quiet) {
				fprintf(stderr,"Stopped at event %d %s\n",i,strerror(errno));
			}
			if (i==0) {
				if (!quiet) printf("No events added\n");
				test_skip(test_string);
			}
			test_pass(test_string);
			exit(1);
		}
	}

	/* enable counting */
	ret=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);
	if (ret<0) {
		if (!quiet) fprintf(stderr,"Events failed at ENABLE rather than at open\n");
		test_kernel_fail(test_string);
	}

	//   naive_matrix_multiply(quiet);

	/* disable counting */
	ret=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	long long buffer;
	int zeros=0;

	for(i=0;i<NUM_EVENTS;i++) {

		result=read(fd[i],&buffer,sizeof(long long));
		if (result<=0) {
			if (!quiet) fprintf(stderr,"Test failed at read time\n");
			test_fail(test_string);
		}

		if (buffer==0) zeros++;
	}

	if (!quiet) fprintf(stderr,"%d counters were zero\n",zeros);

	if (zeros==NUM_EVENTS) {
		if (!quiet) fprintf(stderr,"All counters were zero.\n");
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}
