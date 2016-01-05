/* Test what happens when the size of our attr struct	*/
/* is smaller than the kernel supports.			*/

/* by Vince Weaver, vincent.weaver _at_ maine.edu	*/


static char test_string[]="Testing attr < kernel supported size...";

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

	int header_size;

	int i;
	int already_printed=0;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks what happens if attr struct is "
		      "smaller than kernel expects.\n");
	}

	header_size=sizeof(struct perf_event_attr);

	memset(&pe,0,header_size);

	for(i=-1;i<=header_size;i++) {


		pe.size=i;
		pe.type=PERF_TYPE_HARDWARE;
		pe.config=PERF_COUNT_HW_INSTRUCTIONS;
		pe.disabled=1;
		pe.exclude_kernel=1;
		pe.exclude_hv=1;
		arch_adjust_domain(&pe,quiet);

		fd=perf_event_open(&pe,0,-1,-1,0);
		if (fd<0) {

			if (i==0) {
				fprintf(stderr,"Size of 0 should work!\n");
				test_fail(test_string);
			}

			if ((errno==E2BIG) && (i<PERF_ATTR_SIZE_VER0)) {
				/* expected */
				if ((!quiet) && (!already_printed)) {
					printf("Invalid size (expected), kernel suggests %d\n",pe.size);
					already_printed=1;
				}
			}
			else {
				fprintf(stderr,"Error opening size %d (%d) %s\n",
					i,header_size,strerror(errno));
				test_fail(test_string);
			}
		}

		close(fd);
	}

	test_pass(test_string);

	return 0;
}
