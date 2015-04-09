/* This tests the maximum number of events that can be opened		*/
/* This limit is set by the maximum number of file descriptors		*/
/* Which can be read by */
/*    ulimit -Sn    -- soft limit (1024 on my machine) 			*/
/*    ulimit -Hn    -- hard limit (4096 on my machine)			*/
/*									*/
/* On more recent kernel/system this is now 65536 for both		*/

/* Note: with 65536 on an x86_64 kernel, over 170MB of kernel mem	*/
/*	is allocated.							*/

/* by Vince Weaver, vincent.weaver _at_ maine.edu			*/

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

char test_string[]="Testing max events that can be created...";

#define MAX_LIMIT 100000

/* Declared global, as if you have them local in main() */
/* It exhausts all the stack space and segfaults        */
static struct perf_event_attr pe[MAX_LIMIT];
static int fd[MAX_LIMIT];


int main(int argc, char **argv) {

	int quiet,i;

	quiet=test_quiet();

	for(i=0;i<MAX_LIMIT;i++) {

		if (!quiet) {
			if (i%25==0) printf("Trying event %d\n",i);
		}

		memset(&pe[i],0,sizeof(struct perf_event_attr));
		pe[i].type=PERF_TYPE_HARDWARE;
		pe[i].size=sizeof(struct perf_event_attr);
		pe[i].config=PERF_COUNT_HW_INSTRUCTIONS;
		pe[i].disabled=1;
		pe[i].exclude_kernel=1;
		pe[i].exclude_hv=1;

		fd[i]=perf_event_open(&pe[i],0,-1,-1,0);
		if (fd[i]<0) {
			if (i==0) {
				if (!quiet) {
					printf("could not open event\n");
				}
				test_skip(test_string);
			}

			if (errno==EMFILE) {
				if (!quiet) {
					printf("Ran out of file descriptors at %d %s\n",i,strerror(errno));
				}
				test_pass(test_string);
				exit(0);
			} else {
				if (!quiet) {
					printf("Unexpected error at %d %s\n",i,strerror(errno));
				}
				test_fail(test_string);
				exit(0);

			}
		}

	}

	if (!quiet) {
		printf("Unexpectedly managed to create %d events!\n",i);
	}
	test_fail( test_string );

	return 0;
}


