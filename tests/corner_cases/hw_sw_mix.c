/* hw_sw_mix.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* Tests if mixes of hardware and software events work    */
/* especially for the problem case where the group leader */
/* is not a hardware event.                               */

/* Based on a test by Jiri Olsa */

#define _GNU_SOURCE 1

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


#define EVENTS 4

#define READ_SIZE (EVENTS + 1)

int main(int argc, char** argv) {

	int fd[EVENTS],ret,quiet;
	int result;
	int read_result;
	long long count[READ_SIZE];
	int i,e;

	struct perf_event_attr pe[EVENTS];

	char test_string[]="Testing mixes of HW and SW events...";

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing mixes of HW and SW events if group leader is SW.\n");
		printf("This is known to be broken on all kernels(?)\n");
	}

	if (!quiet) printf("Testing with: ");

	for(e=0;e<(1<<EVENTS);e++) {

		for(i=0;i<EVENTS;i++) {
			memset(&pe[i],0,sizeof(struct perf_event_attr));
			pe[i].size=sizeof(struct perf_event_attr);

			if (e&(1<<i)) {
				pe[i].type=PERF_TYPE_HARDWARE;
				pe[i].config=PERF_COUNT_HW_CPU_CYCLES;
				if (!quiet) printf("H");
			}
			else {
				pe[i].type=PERF_TYPE_SOFTWARE;
				pe[i].config=PERF_COUNT_SW_TASK_CLOCK;
				if (!quiet) printf("S");
			}

			if (i==0) {
				pe[i].disabled=1;
			}

			pe[i].exclude_kernel=1;
			pe[i].read_format=PERF_FORMAT_GROUP;

			fd[i]=perf_event_open(&pe[i],0,-1,i==0?-1:fd[0],0);
			if (fd[0]<0) {
				fprintf(stderr,"Error opening\n");
				test_fail(test_string);
				exit(1);
			}
		}

		if (!quiet) printf("\n");

		ioctl(fd[0], PERF_EVENT_IOC_RESET, 0);
		ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

		result=instructions_million();
		if (result==CODE_UNIMPLEMENTED) {
			test_skip(test_string);
		}

		ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

		read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
		if (read_result!=sizeof(long long)*READ_SIZE) {
			if (!quiet) printf("Unexpected read size e=%d\n",e);

			/* Known to fail at SHSS */
			if (e==2) test_known_kernel_bug(test_string);

			test_fail(test_string);
		}

		if (!quiet) {
			for(i=0;i<count[0];i++) {
				printf("\t%i Counted %lld\n",i,count[1+i]);
			}
		}

		for(i=0;i<EVENTS;i++) {
			if (count[i]==0) {
				if (!quiet) {
					fprintf(stderr,"Counter %d did not start as expected\n",i);
				}
				test_fail(test_string);
			}
		}

		for(i=0;i<EVENTS;i++) {
			close(fd[i]);
		}
	}

	(void) ret;

	test_pass(test_string);

	return 0;
}
