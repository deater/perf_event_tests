/* eopnotsupp.c */
/* Tests to see if EOPNOTSUPP properly returned by perf_event_open() */

/* By Vince Weaver vincent.weaver _at_ maine.edu */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include <errno.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"

int main(int argc, char **argv) {

	int fd;
	int quiet;
	struct perf_event_attr attr;

	char test_string[]="Testing EOPNOTSUPP generation...";

	quiet=test_quiet();

	if (!quiet) {
		printf("This tries to get perf_event_open() to generate\n");
		printf("an EOPNOTSUPP errno.\n\n");
	}

        memset(&attr,0,sizeof(struct perf_event_attr));
        attr.type=PERF_TYPE_SOFTWARE;
	attr.size=sizeof(struct perf_event_attr);
        attr.config=PERF_COUNT_SW_CONTEXT_SWITCHES;
	attr.sample_period=100000;
	attr.sample_type=PERF_SAMPLE_BRANCH_STACK;
	attr.branch_sample_type=PERF_SAMPLE_BRANCH_PLM_ALL|PERF_SAMPLE_BRANCH_ANY;

	attr.read_format=PERF_FORMAT_GROUP |
                PERF_FORMAT_ID |
                PERF_FORMAT_TOTAL_TIME_ENABLED |
                PERF_FORMAT_TOTAL_TIME_RUNNING;

	attr.disabled=1;
	attr.pinned=1;
	attr.exclude_kernel=1;
	attr.exclude_hv=1;
	attr.wakeup_events=1;

	fd=perf_event_open(&attr,
				0, /* current thread */
				-1, /* any cpu */
				-1, /* New Group Leader */
				0 /*0*/ );

	if (fd<0) {
		if (errno==EOPNOTSUPP) {
			if (!quiet) {
				printf("Properly triggered EOPNOTSUPP\n");
			}
			test_pass(test_string);
			exit(0);
		}

	}

	if (!quiet) {
		printf("Unexpectedly got: %s\n",strerror(errno));
	}

	test_fail(test_string);

	return 0;
}
