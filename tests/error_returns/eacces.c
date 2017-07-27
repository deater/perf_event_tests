/* eacces.c */
/* Tests to see if EACCES properly returned by perf_event_open() */

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
	int paranoid;

	char test_string[]="Testing EACCES generation...";

	quiet=test_quiet();

	if (!quiet) {
		printf("This tries to get perf_event_open() to generate\n");
		printf("an EACCES return.\n\n");
	}


	/******************************************/
	/* Check attaching to CPU with any thread */
	/* FIXME: check paranoid value            */
	/*        to avoid false failures         */
	/******************************************/

	if (!quiet) {
		printf("+ Trying to attach to CPU with \"any thread\" set\n");
	}

	paranoid=get_paranoid_setting();
	if (!quiet) printf("\tParanoid = %d\n",paranoid);

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_HARDWARE;
	attr.config=PERF_COUNT_HW_INSTRUCTIONS;

	fd=perf_event_open(&attr,
				-1,	/* any thread */
				0,	/* cpu 0 */
				-1,	/* New Group Leader */
				0	/*0*/ );

	if (fd<0) {

		if (errno==EACCES) {
			if (!quiet) {
				printf("\tProperly triggered EACCES\n");
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
		if (paranoid>0) {
			if (!quiet) {
				printf("\tUnexpectedly opened properly.\n");
			}
			failures++;
		}
		else {
			if (!quiet) {
				printf("\tWas able to open because paranoid<=0\n");
			}
		}
		close(fd);
	}

	/**********************************************/
	/* Check attaching to thread that's not ours  */
	/**********************************************/

	if (!quiet) {
		printf("+ Trying to attach to PID1\n");
		printf("\tparanoid=%d uid=%d\n",paranoid,getuid());
	}

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_HARDWARE;
	attr.config=PERF_COUNT_HW_INSTRUCTIONS;

	fd=perf_event_open(&attr,
				1,	/* init */
				-1,	/* any cpu */
				-1,	/* New Group Leader */
				0	/*0*/ );

	if (fd<0) {
		if (errno==EACCES) {
			if (!quiet) {
				printf("\tProperly triggered EACCES\n");
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
		if (getuid()==0) {
			if (!quiet) {
				printf("\tWas able to open as you are running as root\n");
			}
		}
		else {
			if (!quiet) {
				printf("\tUnexpectedly opened properly.\n");
			}
			failures++;
		}
		close(fd);
	}


	/* Done and report */

	if (failures) {
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}
