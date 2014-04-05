/* einval.c */
/* Tests to see if EINVAL properly returned by perf_event_open() */

/* EINVAL is a catch-all error and so there are many more causes */
/* than we can really test for.					 */

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

	int fd,fd2,failures=0;
	int quiet;
	struct perf_event_attr attr;

	char test_string[]="Testing EINVAL generation...";

	quiet=test_quiet();

	if (!quiet) {
		printf("This tries to get perf_event_open() to generate\n");
		printf("an EINVAL errno.\n\n");
	}


	/******************************************/
	/* Check attaching to an invalid CPU      */
	/******************************************/

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_HARDWARE;
	attr.config=PERF_COUNT_HW_INSTRUCTIONS;

	if (!quiet) {
		printf("Trying invalid CPU: ");
	}

	fd=perf_event_open(&attr,
				-1,	/* any thread */
				-5,	/* invalid CPU */
				-1,	/* New Group Leader */
				0	/*0*/ );

	if (fd<0) {
		if (errno==EINVAL) {
			if (!quiet) {
				printf("Properly triggered EINVAL\n");
			}
		}
		else {
			if (!quiet) {
				printf("Unexpectedly got: %s\n",strerror(errno));
			}
			failures++;
		}
	}
	else {
		if (!quiet) {
			printf("Unexpectedly opened properly.\n");
		}
		failures++;
		close(fd);
	}

	/******************************************/
	/* Check sampling rate too high           */
	/******************************************/

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_HARDWARE;
	attr.config=PERF_COUNT_HW_INSTRUCTIONS;
	attr.sample_freq=0x80000000;
	attr.freq=1;

	if (!quiet) {
		printf("Trying too-high sample rate: ");
	}

	fd=perf_event_open(&attr,
				0,	/* out thread */
				-1,	/* any cpu */
				-1,	/* New Group Leader */
				0	/*0*/ );

	if (fd<0) {
		if (errno==EINVAL) {
			if (!quiet) {
				printf("Properly triggered EINVAL\n");
			}
		}
		else {
			if (!quiet) {
				printf("Unexpectedly got: %s\n",strerror(errno));
			}
			failures++;
		}
	}
	else {
		if (!quiet) {
			printf("Unexpectedly opened properly.\n");
		}
		failures++;
		close(fd);
	}

	/******************************************/
	/* read_format out of range               */
	/******************************************/

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_HARDWARE;
	attr.config=PERF_COUNT_HW_INSTRUCTIONS;
	attr.read_format=0xffffffff;

	if (!quiet) {
		printf("Trying invalid read format: ");
	}

	fd=perf_event_open(&attr,
				0,	/* our pid */
				-1,	/* any cpu */
				-1,	/* New Group Leader */
				0	/*0*/ );

	if (fd<0) {
		if (errno==EINVAL) {
			if (!quiet) {
				printf("Properly triggered EINVAL\n");
			}
		}
		else {
			if (!quiet) {
				printf("Unexpectedly got: %s\n",strerror(errno));
			}
			failures++;
		}
	}
	else {
		if (!quiet) {
			printf("Unexpectedly opened properly.\n");
		}
		failures++;
		close(fd);
	}

	/******************************************/
	/* sample_type out of range               */
	/******************************************/

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_HARDWARE;
	attr.config=PERF_COUNT_HW_INSTRUCTIONS;
	attr.sample_type=0xffffffff;

	if (!quiet) {
		printf("Trying invalid sample type: ");
	}

	fd=perf_event_open(&attr,
				0,	/* our pid */
				-1,	/* any cpu */
				-1,	/* New Group Leader */
				0	/*0*/ );

	if (fd<0) {
		if (errno==EINVAL) {
			if (!quiet) {
				printf("Properly triggered EINVAL\n");
			}
		}
		else {
			if (!quiet) {
				printf("Unexpectedly got: %s\n",strerror(errno));
			}
			failures++;
		}
	}
	else {
		if (!quiet) {
			printf("Unexpectedly opened properly.\n");
		}
		failures++;
		close(fd);
	}


	/******************************************/
	/* flags out of range			  */
	/******************************************/

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_HARDWARE;
	attr.config=PERF_COUNT_HW_INSTRUCTIONS;

	if (!quiet) {
		printf("Trying invalid flags: ");
	}

	fd=perf_event_open(&attr,
				0,	/* our pid */
				-1,	/* any cpu */
				-1,	/* New Group Leader */
				0xffffffff	/*0*/ );

	if (fd<0) {
		if (errno==EINVAL) {
			if (!quiet) {
				printf("Properly triggered EINVAL\n");
			}
		}
		else {
			if (!quiet) {
				printf("Unexpectedly got: %s\n",strerror(errno));
			}
			failures++;
		}
	}
	else {
		if (!quiet) {
			printf("Unexpectedly opened properly.\n");
		}
		failures++;
		close(fd);
	}

	/******************************************/
	/* event out of range			  */
	/******************************************/

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_HARDWARE;
	attr.config=0xffffffff;

	if (!quiet) {
		printf("Trying invalid hw event: ");
	}

	fd=perf_event_open(&attr,
				0,	/* our pid */
				-1,	/* any cpu */
				-1,	/* New Group Leader */
				0	/*0*/ );

	if (fd<0) {
		if (errno==EINVAL) {
			if (!quiet) {
				printf("Properly triggered EINVAL\n");
			}
		}
		else {
			if (!quiet) {
				printf("Unexpectedly got: %s\n",strerror(errno));
			}
			failures++;
		}
	}
	else {
		if (!quiet) {
			printf("Unexpectedly opened properly.\n");
		}
		failures++;
		close(fd);
	}


	/******************************************/
	/* exclusive and pinned but not lead      */
	/******************************************/

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_HARDWARE;
	attr.config=PERF_COUNT_HW_INSTRUCTIONS;
	attr.exclusive=1;
	attr.pinned=1;

	if (!quiet) {
		printf("Trying excusive and pinned while not leader: ");
	}

	fd=perf_event_open(&attr,
				0,	/* our thread */
				-1,	/* any cpu */
				-1,	/* New Group Leader */
				0	/*0*/ );

	if (fd<0) {
		if (!quiet) {
			printf("Unexpectedly failed: %s\n",strerror(errno));
		}
		failures++;
	}
	else {

		memset(&attr,0,sizeof(struct perf_event_attr));
		attr.type=PERF_TYPE_HARDWARE;
		attr.config=PERF_COUNT_HW_INSTRUCTIONS;
		attr.exclusive=1;
		attr.pinned=1;

		fd2=perf_event_open(&attr,
				0,	/* our thread */
				-1,	/* any cpu */
				fd,	/* Group Leader */
				0	/*0*/ );

		if (fd2<0) {
			if (errno==EINVAL) {
				if (!quiet) {
					printf("Properly triggered EINVAL\n");
				}
			}
			else {
				if (!quiet) {
					printf("Unexpectedly got: %s\n",strerror(errno));
				}
				failures++;
			}
		}
		else {
			if (!quiet) {
				printf("Unexpectedly opened properly.\n");
			}
			failures++;
			close(fd2);
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
