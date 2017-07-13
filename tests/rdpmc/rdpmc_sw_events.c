/* This file attempts to see if rdpmc works on sw events   */
/* It shouldn't, but we should fall back in that case	   */

/* by Vince Weaver, vincent.weaver _at_ maine.edu       */

/*
	read = leader->total_time_running
	??
	calc_timer_values
		*enabled = ctx_time - event->tstamp_enabled;

*/

char test_string[]="Testing if rdpmc fallback works on sw events...";
int quiet=0;

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
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <signal.h>

#define MAX_EVENTS 16

int main(int argc, char **argv) {

	int i;
	long page_size=getpagesize();
	double error;

	void *addr[MAX_EVENTS];

	struct perf_event_attr pe;

	int fd[MAX_EVENTS],ret1,ret2;
	int result;

	unsigned long long values[MAX_EVENTS],enabled[MAX_EVENTS],running[MAX_EVENTS];
	unsigned long long values2[MAX_EVENTS],enabled2[MAX_EVENTS],running2[MAX_EVENTS];

	int count=2;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if rdpmc with sw events falls back.\n\n");
	}

	/* See if we support rdpmc access */
	if (!detect_rdpmc(quiet)) {
		test_skip(test_string);
	}

	/*****************************/
	/* TEST START/WORK/READ/STOP */
	/*****************************/

	/* Open event, pid1  */
	memset(&pe,0,sizeof(struct perf_event_attr));


	pe.size=sizeof(struct perf_event_attr);

	pe.read_format=PERF_FORMAT_TOTAL_TIME_ENABLED |
			PERF_FORMAT_TOTAL_TIME_RUNNING;

	fd[0]=-1;

	for(i=0;i<count;i++) {

		if (i==0) {
			pe.type=PERF_TYPE_HARDWARE;
			pe.config=PERF_COUNT_HW_INSTRUCTIONS;
			pe.disabled=1;
			pe.pinned=1;
		}
		else {
			pe.type=PERF_TYPE_SOFTWARE;
			pe.config=PERF_COUNT_SW_TASK_CLOCK;
			pe.disabled=0;
			pe.pinned=0;
		}

		fd[i]=perf_event_open(&pe,0,-1,fd[0],0);
		if (fd[i]<0) {
			fprintf(stderr,"Error opening event %d\n",i);
			test_fail(test_string);
		}

		/* mmap() event */
		addr[i]=mmap(NULL,page_size, PROT_READ, MAP_SHARED,fd[i],0);
		if (addr[i] == MAP_FAILED) {
			fprintf(stderr,"Error mmap()ing event %d!\n",i);
			test_fail(test_string);
		}
	}

	/* start */
	ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	result=instructions_million();
	if (result==CODE_UNIMPLEMENTED) {
		test_skip(test_string);
	}

	long long temp[MAX_EVENTS][64];

	/* read */
	for(i=0;i<count;i++) {
		values[i] = mmap_read_self(addr[i], &enabled[i], &running[i]);
		read(fd[i],temp[i],64);
	}

	/* stop */
	ret2=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	for(i=0;i<count;i++) {
		values2[i]=temp[i][0];
		enabled2[i]=temp[i][1];
		running2[i]=temp[i][2];
	}

	if (ret1<0) {
		fprintf(stderr,"Error starting!\n");
		test_fail(test_string);
	}

	if (ret2<0) {
		fprintf(stderr,"Error stopping!\n");
		test_fail(test_string);
	}

	if (values[0]<0) {
		if (!quiet) printf("rdpmc support not available.\n");
		test_yellow_no(test_string);
	}

	for(i=0;i<count;i++) {
		close(fd[i]);
		munmap(addr[i],page_size);
	}

	if (!quiet) {

		for(i=0;i<count;i++) {
			printf("\t* RDPMC 1 Event %x -- count: %lld enabled %lld running: %lld\n",
				i,values[i],enabled[i],running[i]);
		}

		for(i=0;i<count;i++) {
			printf("\t* READ() 1 Event %x -- count: %lld enabled %lld running: %lld\n",
				i,values2[i],enabled2[i],running2[i]);
		}
	}

	if (!quiet) printf("\n");

	error=display_error(values[0],
				values[0],
				values[0],
				1000000ULL,quiet);

	if ((error>10.0) || ( error<-10.0)) {
		if (!quiet) printf("Error out of range!\n");
		test_fail(test_string);
	}

	for(i=0;i<count;i++) {
		close(fd[i]);
		munmap(addr[i],page_size);
	}

	test_pass(test_string);

	return 0;
}
