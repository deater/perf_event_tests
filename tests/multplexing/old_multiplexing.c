/* check_multiplexing.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* need to document more exactly what this test is doing */

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

#define NUM_EVENTS 2
#define NUM_EVENTSETS 2


int main(int argc, char** argv) {

	int ret,quiet,i,j;

	struct perf_event_attr pe;
	long long buffer[NUM_EVENTS*2+3];

	char test_string[]="Testing if kernel multiplexing works...";

	int fd[NUM_EVENTSETS][NUM_EVENTS];

	long long events[NUM_EVENTSETS][NUM_EVENTS]={
		{
			PERF_COUNT_HW_CPU_CYCLES,
			PERF_COUNT_HW_INSTRUCTIONS,
		},
		{
			PERF_COUNT_HW_INSTRUCTIONS,
			PERF_COUNT_HW_BRANCH_INSTRUCTIONS,
		}
	};

	quiet=test_quiet();

	if (!quiet) {
		printf("Before 2.6.33 kernel multiplexing didn't work.\n");
	}

	for(j=0;j<NUM_EVENTSETS;j++) {

		fd[j][0]=-1;
		for(i=0;i<NUM_EVENTS;i++) {

			memset(&pe,0,sizeof(struct perf_event_attr));
			pe.type=PERF_TYPE_HARDWARE;
			pe.size=sizeof(struct perf_event_attr);
			pe.config=events[j][i];
			if (i==0) {
				pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID|
						PERF_FORMAT_TOTAL_TIME_ENABLED|
						PERF_FORMAT_TOTAL_TIME_RUNNING;
				pe.disabled=1;
				pe.pinned=1;
			}
			pe.exclude_kernel=1;
			pe.exclude_hv=1;

			fd[j][i]=perf_event_open(&pe,0,-1,fd[j][0],0);
			if (fd[j][i]<0) {
				if (!quiet) {
					fprintf(stderr,"Stopped at event %d\n",i);
				}
				test_fail(test_string);
			}
		}
	}

	for(j=0;j<NUM_EVENTSETS;j++) {
		ret=ioctl(fd[j][0], PERF_EVENT_IOC_RESET, 0);
		ret=ioctl(fd[j][0], PERF_EVENT_IOC_ENABLE,0);
		if (ret<0) {
			if (!quiet) {
				fprintf(stderr,"Events failed at ENABLE rather than at open\n");
			}
			test_fail(test_string);
		}
	}

	naive_matrix_multiply(quiet);

	/* disable counting */
	for(j=0;j<NUM_EVENTSETS;j++) {
		ret=ioctl(fd[j][0], PERF_EVENT_IOC_DISABLE,0);
		if (ret<0) {
			if (!quiet) printf("Error disabling\n");
		}
	}

	for(j=0;j<NUM_EVENTSETS;j++) {
		/* 3+(2*nr) */
		ret=read(fd[j][0],&buffer,(3+(2*NUM_EVENTS))*sizeof(long long));
		if (ret!=(3+(2*NUM_EVENTS))*sizeof(long long)) {
			fprintf(stderr,"Weird read value: %d\n",ret);
			test_fail(test_string);
		}
		if (!quiet) {
			printf("Eventset %d\n",j);
			printf("\tEvents: %lld\n",buffer[0]);
			printf("\tTime Enabled: %lld\n",buffer[1]);
			printf("\tTime Running: %lld\n",buffer[2]);
			for(i=0;i<buffer[0];i++) {
				printf("\tEvent %d id %lld value %lld\n",i,
		   			buffer[3+1+(i*2)],
					buffer[3+(i*2)]);
			}
		}
	}

	test_pass(test_string);

	return 0;
}

