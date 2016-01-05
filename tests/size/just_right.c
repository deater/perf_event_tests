/* Test what happens when the size is what the kerel expects	*/
/* This should give very boring results.			*/

/* by Vince Weaver, vincent.weaver _at_ maine.edu		*/


static char test_string[]="Testing attr is kernel supported size...";

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

	int kernel_size;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks what happens if attr struct is "
		      "exactly what the kernel expects.\n");
	}

	kernel_size=sizeof(struct perf_event_attr);

	memset(&pe,0,kernel_size);
	pe.size=kernel_size;
	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		fprintf(stderr,"Error opening leader %llx %s\n",
			pe.config,strerror(errno));
		test_fail(test_string);
	}

	close(fd);

	test_pass(test_string);

	return 0;
}

