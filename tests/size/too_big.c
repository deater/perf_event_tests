/* Test what happens when the size of our attr struct	*/
/* is larger than the kernel supports.			*/

/* by Vince Weaver, vincent.weaver _at_ maine.edu	*/


static char test_string[]="Testing attr > kernel supported size...";

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

	struct perf_event_attr *pe;
	char *data;

	int fd;
	int quiet=0;
	int i;

	int kernel_size;
	int actual_size;
	int already_printed=0;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks what happens if attr struct is "
		      "bigger than kernel expects.\n");
	}

	data=malloc(2*getpagesize());
	pe=(struct perf_event_attr *)data;

	kernel_size=sizeof(struct perf_event_attr);

	/*************************************/
	/* First try with extra space zeroed */
	/*************************************/

	for(i=kernel_size;i<2*getpagesize();i++) {
		/* She's actual size but she seems much bigger to me.  --TMBG */
		actual_size=i;

		memset(pe,0,actual_size);
		pe->size=actual_size;
		pe->type=PERF_TYPE_HARDWARE;
		pe->config=PERF_COUNT_HW_INSTRUCTIONS;
		pe->disabled=1;
		pe->exclude_kernel=1;
		pe->exclude_hv=1;

		arch_adjust_domain(pe,quiet);

		fd=perf_event_open(pe,0,-1,-1,0);
		if (fd<0) {
			if ((errno==E2BIG) && (i>getpagesize())) {
				/* expected */

				if ((!quiet) && (!already_printed)) {
					printf("Invalid size (expected), kernel suggests %d\n",pe->size);
					already_printed=1;
				}
			}
			else {
				fprintf(stderr,"Error opening, size=%d  %s\n",
					i,strerror(errno));
				test_fail(test_string);
			}
		}



		close(fd);
	}


	/*************************************/
	/* Next try with extra space ones    */
	/*************************************/

	for(i=kernel_size;i<2*getpagesize();i++) {
		data[i]=0xff;
	}

	for(i=kernel_size;i<2*getpagesize();i++) {
		/* She's actual size but she seems much bigger to me.  --TMBG */
		actual_size=i;

		memset(pe,0,kernel_size);
		pe->size=actual_size;
		pe->type=PERF_TYPE_HARDWARE;
		pe->config=PERF_COUNT_HW_INSTRUCTIONS;
		pe->disabled=1;
		pe->exclude_kernel=1;
		pe->exclude_hv=1;

		arch_adjust_domain(pe,quiet);

		fd=perf_event_open(pe,0,-1,-1,0);
		if (fd<0) {
			if ((errno==E2BIG) && (i>getpagesize())) {
				/* expected */

				if ((!quiet) && (!already_printed)) {
					printf("Invalid size (expected), kernel suggests %d\n",pe->size);
					already_printed=1;
				}
			}
			else if ((errno==E2BIG) && (data[i]=0xff)) {
				/* expected if bigger than kernel expects */
				/* and filled with non-zeros */
			}
			else {
				fprintf(stderr,"Error opening, size=%d %s\n",
					i,strerror(errno));
				test_fail(test_string);
			}
		}



		close(fd);
	}

	test_pass(test_string);

	return 0;
}
