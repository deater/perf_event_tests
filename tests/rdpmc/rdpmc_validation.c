/* This file attempts to validate rdpmc counter access  */
/* This feature was introduced with Linux 3.4           */

/* by Vince Weaver, vincent.weaver _at_ maine.edu       */


char test_string[]="Testing if userspace rdpmc reads give expected results...";
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


#define MAX_EVENTS 16


int main(int argc, char **argv) {

	int i,result;
	long page_size=getpagesize();
	double error;

	long long start_before,stop_after;

	void *addr[MAX_EVENTS];

	struct perf_event_attr pe;

	int fd[MAX_EVENTS],ret1,ret2;

	unsigned long long values[MAX_EVENTS],enabled[MAX_EVENTS],running[MAX_EVENTS];
	unsigned long long values2[MAX_EVENTS],enabled2[MAX_EVENTS],running2[MAX_EVENTS];

	int count=2;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if userspace rdpmc() style reads work.\n\n");
	}

	/* See if we support rdpmc access */
	if (!detect_rdpmc(quiet)) {
		test_skip(test_string);
	}


	/*****************************/
	/* TEST START/WORK/READ/STOP */
	/*****************************/

	/* Open event  */
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

		/* mmap() event */
		addr[i]=mmap(NULL,page_size, PROT_READ, MAP_SHARED,fd[i],0);
		if (addr[i] == (void *)(-1)) {
			fprintf(stderr,"Error mmap()ing event %d!\n",i);
			test_fail(test_string);
		}
	}


	/* start */
	start_before=rdtsc();

	ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	result=instructions_million();
	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

	/* read */
	for(i=0;i<count;i++) {
		values[i] = mmap_read_self(addr[i], &enabled[i], &running[i]);
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

	if (values[0]<0) {
		if (!quiet) printf("rdpmc support not available.\n");
		test_yellow_no(test_string);
	}

	if (!quiet) {
		printf("total start/read/stop latency: %lld cycles\n",
			stop_after-start_before);
		for(i=0;i<count;i++) {
			printf("\tEvent %x -- count: %lld enabled: %lld running: %lld\n",
				i,values[i],enabled[i],running[i]);
		}
	}

	for(i=0;i<count;i++) {
		close(fd[i]);
		munmap(addr[i],page_size);
	}

	/**********************************/
	/* TEST START/READ/WORK/READ/STOP */
	/**********************************/

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
		if (addr[i] == (void *)(-1)) {
			fprintf(stderr,"Error mmap()ing event %d!\n",i);
			test_fail(test_string);
		}
	}

	/* start */
	start_before=rdtsc();

	ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	for(i=0;i<count;i++) {
		values[i] = mmap_read_self(addr[i], &enabled[i], &running[i]);
	}

	result=instructions_million();
	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

	/* read */
	for(i=0;i<count;i++) {
		values2[i] = mmap_read_self(addr[i],&enabled2[i],&running2[i]);
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

	if (values[0]<0) {
		if (!quiet) printf("rdpmc support not available.\n");
		test_yellow_no(test_string);
	}

	if (!quiet) {
		printf("total start/read/work/read/stop latency: %lld cycles\n",
			stop_after-start_before);
		for(i=0;i<count;i++) {
			printf("\t* Event %x -- count: %lld enabled %lld running: %lld\n",
				i,values[i],enabled[i],running[i]);
			printf("\t! Event %x -- count: %lld enabled %lld running: %lld\n",
				i,values2[i],enabled2[i],running2[i]);
		}
	}

	if (!quiet) printf("\n");

	error=display_error(values2[0]-values[0],
				values2[0]-values[0],
				values2[0]-values[0],
				1000000ULL,quiet);

	if ((error>1.0) || ( error<-1.0)) {
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
