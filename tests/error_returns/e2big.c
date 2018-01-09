/* e2big.c */
/* Tests to see if E2BIG properly returned by perf_event_open() */

/* By Vince Weaver vincent.weaver _at_ maine.edu */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

#include <errno.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"

int main(int argc, char **argv) {

	int fd,i;
	struct perf_event_attr *pe;
	char *pointer;
	int errors=0;
	int quiet;
	int expected_size=0;

	char test_string[]="Testing E2BIG errors...";

	quiet=test_quiet();

	if (!quiet) {
		printf("This tests various size values and checks that\n");
		printf("E2BIG is properly returned.\n\n");
	}

	pointer=calloc(getpagesize()*3,sizeof(char));

	pe=(struct perf_event_attr *)pointer;

	for(i=0;i<getpagesize()*2;i++) {

		memset(pointer,0,getpagesize()*3);

		pe->type=PERF_TYPE_HARDWARE;
		pe->size=i;
		pe->config=PERF_COUNT_HW_INSTRUCTIONS;

		fd=perf_event_open(pe,
				0, /* current thread */
				-1, /* any cpu */
				-1, /* New Group Leader */
				0 /*0*/ );

		if (fd<0) {
			if (errno==E2BIG) {
				if ((i==0) ||
					((i>=PERF_ATTR_SIZE_VER0) &&
					 (i<=getpagesize()))) {
					if (!quiet) {
						printf("Unexpected failure with size %d\n",i);
					}
					errors++;
				}
				expected_size=pe->size;

			}
			else {
				if (!quiet) {
					printf("Unknown failure with size %d: %s\n",
						i,strerror(errno));
				}
				errors++;
			}

		}
		else {
			if ((i==0) ||
				((i>=PERF_ATTR_SIZE_VER0) &&
				 (i<=getpagesize()))) {
					/* good */
			} else {
				if (!quiet) {
					printf("Unexpected success with size %d\n",i);
				}
				errors++;
			}
			close(fd);
		}

	}


//	printf("Expected: %d\n",expected_size);

	/* Check if it works with too big size but garbage after */

	pe->size=expected_size+8;
	pointer[expected_size+4]=0xff;

	fd=perf_event_open(pe,
			0, /* current thread */
			-1, /* any cpu */
			-1, /* New Group Leader */
			0 /*0*/ );

	if (fd<0) {
		if (errno==E2BIG) {
			/* good */
		}
		else {
			printf("Unexpected error: %s\n",strerror(errno));
			errors++;
		}
	}
	else {
		if (!quiet) {
			printf("Unexpected success when non-zero present\n");
			printf("Past end of file!\n");
		}
		errors++;
		close(fd);
	}

	if (errors) {
		test_fail(test_string);
	}
	else {
		test_pass(test_string);
	}
	return 0;
}
