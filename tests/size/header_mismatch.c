/* Detect when the built-in kernel size does not match the size	*/
/* of the header we are using.					*/

/* by Vince Weaver, vincent.weaver _at_ maine.edu	*/


static char test_string[]="Testing header vs kernel attr size...";

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"

int main(int argc, char **argv) {

	struct perf_event_attr pe;
	int fd;
	int quiet=0;

	int header_size,kernel_size=0;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if the kernel and header attr sizes match\n");
	}

	header_size=sizeof(struct perf_event_attr);

	memset(&pe,0,header_size);

	pe.size=-1;
	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		if (errno==E2BIG) {
			kernel_size=pe.size;
		}

		else {
			printf("Unexpected errors %s\n",strerror(errno));
			test_fail(test_string);
		}
	}
	else {
		printf("Unexpectedly opened with %d (%d) size\n",-1, pe.size);
		close(fd);
		test_fail(test_string);
	}

	if (kernel_size!=header_size) {
		if (!quiet) {
			printf("Kernel expects %d but header is %d\n",
				kernel_size,header_size);
		}
		test_caution(test_string);
	}

	test_pass(test_string);

	return 0;
}
