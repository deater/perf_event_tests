/* flags_range_check.c  */
/* Test if unsupported flags values are properly rejected	*/

/* Before 3.15 on 64-bit machines the top 32 bits were not 	*/
/* properly checked.						*/

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

int main(int argc, char** argv) {

	int fd1,quiet;

	long flags;

	struct perf_event_attr pe;

	char test_string[]="Testing flags invalid bits...";

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing flags invalid bits.\n");
		printf("This was broken prior to Linux 3.15\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	/* test 1, 0x40000000 */

	fd1=perf_event_open(&pe,0,-1,-1,0x4000000);
	if (fd1<0) {

		if (errno==EINVAL) {
			if (!quiet) {
				printf("Correctly failed opening leader with flags %x\n",0x40000000);
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
			fprintf(stderr,"Unexpectedly opened properly with flags %x\n",0x40000000);
		}
		test_fail(test_string);
	}


	/* test 1, was 0x800000000 */
	/* caused warnings on 32-bit systems */
	/* Hopefully high bit being set is equivelent? */

	flags = 1UL<<((sizeof(long)*8)-1);

	if (!quiet) printf("Using %lx\n",flags);

	fd1=perf_event_open(&pe,0,-1,-1,flags);
	if (fd1<0) {
		if (errno==EINVAL) {
			if (!quiet) {
				printf("Correctly failed opening leader with flags %lx\n",flags);
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
		int version;

		close(fd1);

		version=get_kernel_version();

		if (version<0x30f00) {

			if (!quiet) {
				fprintf(stderr,"Unexpectedly opened properly with flags %lx\n",flags);
				fprintf(stderr,"This was not fixed until Linux 3.15\n");
			}
			test_fail_kernel(test_string);
		} else {
			if (!quiet) {
				fprintf(stderr,"Unexpectedly opened properly with flags %lx\n",flags);
			}
			test_fail(test_string);
		}
	}


	test_pass(test_string);

	return 0;
}

