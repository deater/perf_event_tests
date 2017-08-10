/* tracepoint_alias.c					*/
/* by Vince Weaver <vincent.weaver _at_ maine.edu	*/

/* Tests if tracepoints alias if the high 32 bits set	*/
/* This bug existed through at least 3.12		*/

#define _GNU_SOURCE 1
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

//#include <linux/hw_breakpoint.h>

#include "perf_event.h"

#include "test_utils.h"
#include "perf_helpers.h"



int main(int argc, char **argv) {

	int i;
	int fd;
	struct perf_event_attr pe;
	int found=0;
	int quiet;

	char test_string[]="Testing if tracepoint event ids alias...";

	quiet=test_quiet();

	if (!quiet) {
		printf("Finding a valid tracepoint\n");
	}

	for(i=0;i<1024;i++) {

		memset(&pe,0,sizeof(struct perf_event_attr));
		pe.type=PERF_TYPE_TRACEPOINT;
		pe.size=80;

		pe.config=i;

		fd=perf_event_open(&pe,
				0, /* current thread */
				-1, /* all cpus */
				-1, /* New Group Leader */
				0);


		if (fd<0) {
		}
		else {
			found=1;
			if (!quiet) printf("\tFound %d (0x%x)\n",i,i);
			break;
		}
	}

	if (found) {
		if (!quiet) printf("Trying alias\n");

		close(fd);

		pe.config|=0xffffffff00000000ULL;
		fd=perf_event_open(&pe,
				0, /* current thread */
				-1, /* all cpus */
				-1, /* New Group Leader */
				0);


		if (fd<0) {
			if (!quiet) printf("\tNo alias!  New behavior\n");
			test_pass(test_string);
		}
		else {
			if (!quiet) printf("\t0x%llx aliases to %llx\n",
				(pe.config&0xffffffffULL),pe.config);
			test_yellow_old_behavior(test_string);


		}

	} else {

		if (!quiet) printf("\tNone found\n");
		test_skip(test_string);
	}

	return 0;
}
