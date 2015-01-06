/* flags_cgroup.c  */
/* Test cgroup functionality */

/* Note, your cgroup has to be mounted with the "perf_event"	*/
/*   option for this to work.					*/

/* by Vince Weaver   vincent.weaver@maine.edu */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include <signal.h>

#include <sys/mman.h>

#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/prctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#define CGROUP_FILENAME "/sys/fs/cgroup/systemd/"

int main(int argc, char** argv) {

	int fd1,quiet;

	struct perf_event_attr pe;

	int cgroup_fd;

	char test_string[]="Testing PERF_FLAG_PID_CGROUP flag...";

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing PERF_FLAG_PID_CGROUP flag, ");
		printf("Introuduced in 2.6.39\n");
	}

	cgroup_fd=open(CGROUP_FILENAME,O_RDONLY);
	if (cgroup_fd<1) {
		if (!quiet) {
			fprintf(stderr,"Error opening %s: %s\n",
				CGROUP_FILENAME,
				strerror(errno));
		}
		test_skip(test_string);
	}


	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	/* test 1, invalid cgroup */

	if (!quiet) {
		printf("1. Testing invalid cgroup: ");
	}


	fd1=perf_event_open(&pe,0,-1,-1,PERF_FLAG_PID_CGROUP);
	if (fd1<0) {
		if (errno==EINVAL) {
			if (!quiet) {
				printf("Correctly failed\n");
			}
		}
		else {
			if (!quiet) {
				fprintf(stderr,"Unexpected error %s\n",strerror(errno));
				test_fail(test_string);
			}
		}
	}
	else {
		close(fd1);
		if (!quiet) {
			fprintf(stderr,"Unexpectedly opened properly with invalid cgroup\n");
		}
		test_fail(test_string);
	}


	/* test 2, valid cgroup */

	if (!quiet) {
		printf("2. Testing %s: ",CGROUP_FILENAME);
	}

	fd1=perf_event_open(&pe,cgroup_fd,0,-1,PERF_FLAG_PID_CGROUP);
	if (fd1<0) {

		if (errno==EACCES) {
			fprintf(stderr,"Need higher permissions for access\n");
			test_skip(test_string);
		}

		if (!quiet) {
			fprintf(stderr,"Unexpected error %s\n",strerror(errno));
			test_fail(test_string);
		}
	}
	else {
		if (!quiet) {
			printf("Success!\n");
		}
	}

	close(fd1);

	test_pass(test_string);

	return 0;
}

