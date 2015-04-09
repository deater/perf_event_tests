/* This tests the maximum number of events that can be multiplexed */

/* by Vince Weaver, vincent.weaver _at_ maine.edu                  */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"
#include "instructions_testcode.h"

char test_string[]="Testing limit of multiplexing...";

#define MAX_LIMIT 1024

/* Global, as it takes up too much room on stack */
static struct perf_event_attr pe[MAX_LIMIT];
static int fd[MAX_LIMIT];
static long long insn_count[MAX_LIMIT];

int main(int argc, char **argv) {

	int ret,quiet,i,j;

	quiet=test_quiet();

	for(i=0;i<MAX_LIMIT;i++) {

		if (!quiet) {
			if (i%25==0) printf("Now checking %d\n",i);
		}

		for(j=0;j<i;j++) {

			memset(&pe[j],0,sizeof(struct perf_event_attr));
			pe[j].type=PERF_TYPE_HARDWARE;
			pe[j].size=sizeof(struct perf_event_attr);
			pe[j].config=PERF_COUNT_HW_INSTRUCTIONS;
			pe[j].disabled=1;
			pe[j].exclude_kernel=1;
			pe[j].exclude_hv=1;

			fd[j]=perf_event_open(&pe[j],0,-1,-1,0);
			if (fd[j]<0) {
				if (j==0) {
					if (!quiet) printf("could not open event\n");

					test_skip(test_string);
				}
				if (!quiet) printf("Ran out of file descriptors at %d\n",j);
				test_pass(test_string);
				exit(0);
			}
		}

		for(j=0;j<i;j++) {
			ret=ioctl(fd[j], PERF_EVENT_IOC_ENABLE,0);
			if (ret<0) {
				if (!quiet) fprintf(stderr,"Error enabling event %d\n",j);
				test_fail(test_string);
			}
		}

		instructions_million();

		for(j=0;j<i;j++) {

			ret=ioctl(fd[j], PERF_EVENT_IOC_DISABLE,0);
			if (ret<0) {
				if (!quiet) fprintf(stderr,"Error disabling event %d\n",j);
				test_fail(test_string);
			}
		}

		for(j=0;j<i;j++) {
			ret=read(fd[j],&insn_count[j],sizeof(long long));
			if (ret!=sizeof(long long)) {
				if (!quiet) fprintf(stderr,
					"Unepxected result from read fd[%d]: %d\n",
					j,ret);
				test_fail(test_string);
			}
		}

		if (!quiet) {
			// for(j=0;j<i;j++) printf("\tinsn_count[%d]: %lld\n",j,insn_count[j]);
		}

		for(j=0;j<i;j++) close(fd[j]);
	}

	if (!quiet) {
		fprintf(stderr,"Successfully multiplexed %d events\n",
			MAX_LIMIT);
	}

	test_pass( test_string );

	return 0;
}


