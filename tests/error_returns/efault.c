/* efault.c */
/* Tests to see if EFAULT properly returned by perf_event_open() */

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

	char test_string[]="Testing EFAULT generation...";

	quiet=test_quiet();

	if (!quiet) {
		printf("This tries to get perf_event_open() to generate\n");
		printf("an EFAULT return.\n\n");
	}

	fd=perf_event_open(NULL,
				0, /* current thread */
				-1, /* any cpu */
				-1, /* New Group Leader */
				0 /*0*/ );

	if (fd<0) {
		if (errno==EFAULT) {
			if (!quiet) {
				printf("Properly triggered EFAULT\n");
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
