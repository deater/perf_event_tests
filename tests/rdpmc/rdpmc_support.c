/* This file attempts to test userspace rdpmc counter access   */

/* This feature was introduced in Linux 3.4                    */
/* There were bugs in the detection interface until Linux 3.12 */
/*   with a major API/ABI change in 3.12                       */

/* by Vince Weaver, vincent.weaver _at_ maine.edu              */


static char test_string[]="Testing if userspace rdpmc reads are supported...";
static int quiet=0;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#include "rdpmc_lib.h"

#include <sys/mman.h>


#define MAX_EVENTS 16


int main(int argc, char **argv) {

	int i;
	long page_size=getpagesize();

	long long start_before,stop_after;

	void *addr[MAX_EVENTS];

	struct perf_event_attr pe;
	int fd[MAX_EVENTS],ret1,ret2;
//	struct perf_event_mmap_page *our_mmap;
	int rdpmc_support=0;

	int count=1;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if userspace rdpmc() style reads work.\n\n");
	}


	/* detect support */

	rdpmc_support=detect_rdpmc(quiet);

	if (rdpmc_support==0) {
		test_skip(test_string);
	}

	/***************/
	/* SIMPLE TEST */
	/***************/


	/* open */

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);

	fd[0]=-1;

	for(i=0;i<count;i++) {
		pe.config=PERF_COUNT_HW_INSTRUCTIONS;

		if (i==0) {
			pe.disabled=1;
			pe.pinned=1;
		}
		else {
			pe.disabled=0;
			pe.pinned=0;
		}

		fd[i]=perf_event_open(&pe,0,-1,fd[0],0);
		if (fd[i]<0) {
			fprintf(stderr,"Error opening event %d\n",i);
			test_fail(test_string);
		}

		addr[i]=mmap(NULL,page_size, PROT_READ, MAP_SHARED,fd[i],0);
		if (addr[i] == MAP_FAILED) {
			fprintf(stderr,"Error mmap()ing event %d!\n",i);
			test_fail(test_string);
		}

//		our_mmap=(struct perf_event_mmap_page *)addr[i];

	}

	/* start */

	start_before=rdtsc();

	ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	/* read */

	unsigned long long stamp[MAX_EVENTS],stamp2[MAX_EVENTS];
	unsigned long long now[MAX_EVENTS],now2[MAX_EVENTS];

	for(i=0;i<count;i++) {
		stamp[i] = mmap_read_self(addr[i], NULL, &stamp2[i]);
		now[i] = mmap_read_self(addr[i],NULL,&now2[i]);
	}

	/* stop */
	ret2=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	stop_after=rdtsc();

	if (ret1<0) {
		fprintf(stderr,"Error starting!\n");
		test_fail(test_string);
	}

	if (ret2<0) {
		fprintf(stderr,"Error stopping!\n");
		test_fail(test_string);
	}

	if (stamp[0]<0) {
		if (!quiet) printf("rdpmc support not available.\n");
		test_yellow_no(test_string);
	}

	if (!quiet) {
		printf("total start/read/stop latency: %lld cycles\n",
			stop_after-start_before);
		for(i=0;i<count;i++) {
			printf("\tEvent %x -- count: %lld running: %lld\n",
				i,now[i]-stamp[i],now2[i]-stamp2[i]);
		}
	}

	for(i=0;i<count;i++) {
		close(fd[i]);
		munmap(addr[i],page_size);
	}

	if (rdpmc_support==1) {
		if (!quiet) printf("New behavior\n");
	}
	else {
		test_yellow_old_behavior(test_string);
	}

	test_pass(test_string);

	return 0;
}
