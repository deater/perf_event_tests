/* This tests the maximum number of events that can be opened  */
/* This limit is set by the maximum number of file descriptors */
/* Which can be read by */
/*    ulimit -Sn    -- soft limit (1024 on my machine) */
/*    ulimit -Hn    -- hard limit (4096 on my machine) */

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

char test_string[]="Testing if EMFILE can be triggered...";

/* Typically 1024 is enough */
#define MAX_LIMIT 10000

int main(int argc, char **argv) {

	int quiet,i;
	struct perf_event_attr pe[MAX_LIMIT];
	int fd[MAX_LIMIT];

	quiet=test_quiet();

	for(i=0;i<=MAX_LIMIT;i++) {

		if (!quiet) {
			printf("Trying event %d\n",i);
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


