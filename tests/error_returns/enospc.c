/* enospc.c */
/* Tests to see if ENOSPC properly returned by perf_event_open() */

/* By Vince Weaver vincent.weaver _at_ maine.edu */

/* Prior to Linux 3.3 many of what are now won't fit EINVAL errors */
/* Were returned as ENOSPC, but Linus complained */
/* See aa2bc1ade59003a379ffc485d6da2d92ea3370a6	 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include <errno.h>

#include "perf_event.h"
#include "hw_breakpoint.h"

#include "perf_helpers.h"
#include "test_utils.h"

#define MAX_TRIES 100

int main(int argc, char **argv) {

	int i,fd[MAX_TRIES];
	int quiet;
	struct perf_event_attr attr;

	char test_string[]="Testing ENOSPC generation...";

	quiet=test_quiet();

	if (!quiet) {
		printf("This tries to get perf_event_open() to generate\n");
		printf("an ENOSPC errno.\n\n");
	}

	for(i=0;i<MAX_TRIES;i++) {
	        memset(&attr,0,sizeof(struct perf_event_attr));

		/* This might only work on x86 */
		attr.config=0;
		attr.size=sizeof(struct perf_event_attr);
        	attr.type=PERF_TYPE_BREAKPOINT;
		attr.bp_type=HW_BREAKPOINT_X;
		attr.bp_addr=(unsigned long)main;
		attr.bp_len=sizeof(long);
		attr.exclude_kernel=1;

		fd[i]=perf_event_open(&attr,
				0, /* current thread */
				-1, /* any cpu */
				-1, /* New Group Leader */
				0 /*0*/ );

		if (fd[i]<0) {
			if (errno==ENOSPC) {
				if (!quiet) {
					printf("Properly triggered ENOSPC\n");
				}
				test_pass(test_string);
				exit(0);
			}

		}

	}

	if (!quiet) {
		printf("Unexpectedly got: %s\n",strerror(errno));
	}

	test_fail(test_string);

	return 0;
}
