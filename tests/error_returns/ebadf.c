/* ebadf.c */
/* Tests to see if EBADF properly returned by perf_event_open() */

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

	int fd,failures=0;
	int quiet;
	struct perf_event_attr attr;

	char test_string[]="Testing EBADF generation...";

	quiet=test_quiet();

	if (!quiet) {
		printf("This tries to get perf_event_open() to generate\n");
		printf("an EBADF errno.\n\n");
	}


	/******************************************/
	/* Check using invalid group leader       */
	/******************************************/

	if (!quiet) {
		printf("Trying with invalid group leader\n");
	}

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_HARDWARE;
	attr.config=PERF_COUNT_HW_INSTRUCTIONS;

	fd=perf_event_open(&attr,
				-1,	/* any thread */
				0,	/* cpu 0 */
				-2,	/* New Group Leader */
				0	/*0*/ );

	if (fd<0) {
		if (errno==EBADF) {
			if (!quiet) {
				printf("\tProperly triggered EBADF\n");
			}
		}
		else {
			if (!quiet) {
				printf("\tUnexpectedly got: %s\n",strerror(errno));
			}
			failures++;
		}
	}
	else {
		if (!quiet) {
			printf("\tUnexpectedly opened properly.\n");
		}
		failures++;
		close(fd);
	}

	/**********************************************/
	/* Check invalid cgroup ID                    */
	/**********************************************/

	if (!quiet) {
		printf("Trying with invalid cgroup id\n");
	}

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_HARDWARE;
	attr.config=PERF_COUNT_HW_INSTRUCTIONS;

	fd=perf_event_open(&attr,
				-2,	/* invalid cgroup */
				0,	/* cpu 0 */
				-1,	/* New Group Leader */
				PERF_FLAG_PID_CGROUP);

	if (fd<0) {
		if (errno==EBADF) {
			if (!quiet) {
				printf("\tProperly triggered EBADF\n");
			}
		}
		else if (errno==EINVAL) {
			if (!quiet) {
				printf("\tGot EINVAL, probably because "
					"CONF_CGROUP_PERF not set.\n");
			}
		}
	}
	else {
		if (!quiet) {
			printf("\tUnexpectedly opened properly.\n");
		}
		failures++;
		close(fd);
	}


	/* Done and report */

	if (failures) {
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}
