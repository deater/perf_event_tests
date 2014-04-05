/* enoent.c */
/* Tests to see if ENOENT properly returned by perf_event_open() */

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

	char test_string[]="Testing ENOENT generation...";

	quiet=test_quiet();

	if (!quiet) {
		printf("This tries to get perf_event_open() to generate\n");
		printf("an ENOENT errno.\n\n");
	}

        memset(&attr,0,sizeof(struct perf_event_attr));
        attr.type=-5;
        attr.config=PERF_COUNT_HW_INSTRUCTIONS;

	fd=perf_event_open(&attr,
				0, /* current thread */
				-1, /* any cpu */
				-1, /* New Group Leader */
				0 /*0*/ );

	if (fd<0) {
		if (errno==ENOENT) {
			if (!quiet) {
				printf("Properly triggered ENOENT\n");
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
