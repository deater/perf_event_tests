/* mmap_multiple.c  */
/* by Vince Weaver   <vincent.weaver@maine.edu> */

/* On x86 you can mmap() a perf_event_open() fd multiple times */

/* This doesn't seem to work on ARM64 */

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

#include <sys/ptrace.h>
#include <sys/wait.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"
#include "parse_record.h"

#define SAMPLE_FREQUENCY 1000000000

#define MMAP_DATA_SIZE 8

int main(int argc, char **argv) {

	int fd;
	int mmap_pages=1+MMAP_DATA_SIZE;
	int quiet;
	char *our_mmap,*our_mmap2;

	struct perf_event_attr pe;

	char test_string[]="Trying to mmap same perf_event fd multiple times...";

	quiet=test_quiet();

        /* Set up Instruction Event */

        memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.size=sizeof(struct perf_event_attr);
	pe.sample_period=SAMPLE_FREQUENCY;

	pe.read_format=0;
	pe.disabled=1;
	pe.pinned=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.wakeup_events=1;

	pe.mmap=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		if (!quiet) {
			printf("Problem opening leader %s\n",
				strerror(errno));
		}

		test_fail(test_string);
	}

	our_mmap=mmap(NULL, mmap_pages*getpagesize(),
		PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (our_mmap==MAP_FAILED) {
		if (!quiet) {
			printf("mmap() failed %s!\n",strerror(errno));
		}
		test_fail(test_string);
	}

	our_mmap2=mmap(NULL, mmap_pages*getpagesize(),
		PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (our_mmap2==MAP_FAILED) {
		if (!quiet) {
			printf("mmap() failed %s!\n",strerror(errno));
		}
		test_fail(test_string);
	}


	test_pass(test_string);

	return 0;
}
