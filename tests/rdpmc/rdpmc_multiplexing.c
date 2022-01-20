/* This file attempts to use rdpmc with multiplexing  */

/* by Vince Weaver, vincent.weaver _at_ maine.edu       */

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

	long long start_before,stop_after;

	void *addr[MAX_EVENTS];

	struct perf_event_attr pe;

	int fd[MAX_EVENTS],ret1,ret2;

	unsigned long long values[MAX_EVENTS],enabled[MAX_EVENTS],running[MAX_EVENTS];
	unsigned long long adjusted[MAX_EVENTS];
	int failed[MAX_EVENTS];
	double error;

	int count=10;

	char test_string[]="Testing if userspace rdpmc multiplexing works...";
	int quiet=0;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if userspace rdpmc multiplexing works.\n\n");
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
	pe.config1=ARCH_EVENT_CONFIG1_VAL;
	pe.size=sizeof(struct perf_event_attr);
	pe.read_format=PERF_FORMAT_TOTAL_TIME_ENABLED |
		PERF_FORMAT_TOTAL_TIME_RUNNING;

	fd[0]=-1;

	for(i=0;i<count;i++) {
		pe.config=PERF_COUNT_HW_INSTRUCTIONS;
		pe.disabled=1;

		fd[i]=perf_event_open(&pe,0,-1,-1,0);
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

	for(i=0;i<count;i++) {
		ret1=ioctl(fd[i], PERF_EVENT_IOC_ENABLE,0);
	}

	for(i=0;i<1000;i++) {
		result=instructions_million();
	}

	if (result==CODE_UNIMPLEMENTED) {
		if (!quiet) printf("Warning, no million\n");
		test_skip(test_string);
	}

	/* read */
	for(i=0;i<count;i++) {
		values[i] = mmap_read_self(addr[i], &enabled[i], &running[i]);
	}

	/* stop */
	for(i=0;i<count;i++) {
		ret2=ioctl(fd[i], PERF_EVENT_IOC_DISABLE,0);
	}

	stop_after=rdtsc();

	if (ret1<0) {
		fprintf(stderr,"Error starting!\n");
		test_fail(test_string);
	}

	if (ret2<0) {
		fprintf(stderr,"Error stopping!\n");
		test_fail(test_string);
	}

	for(i=0;i<count;i++) {
		if (values[i]==-1) {
			// fallback to read
			long long temp[MAX_EVENTS][64];

			failed[i]=1;
			result=read(fd[i],temp,3*8);
			if (result!=3*8) {
				printf("Fallback failed!\n");
			}
			values[i]=temp[i][0];
	                enabled[i]=temp[i][1];
        	        running[i]=temp[i][2];
		}
		else {
			failed[i]=0;
		}
	}

	for(i=0;i<count;i++) {
		/* PAPI algo for scaling */
		if (running[i]!=0) {
			adjusted[i] = (enabled[i] * 128LL) / running[i];
			adjusted[i] = adjusted[i]*values[i];
			adjusted[i] = adjusted[i] / 128LL;
		} else {
			adjusted[i]=0;
		}
	}

	if (!quiet) {
		printf("total start/read/stop latency: %lld cycles\n",
			stop_after-start_before);
		for(i=0;i<count;i++) {
			printf("\t");
			if (failed[i]) {
				printf("FALLBACK: ");
			}
			printf("Event %x -- Raw count: %lld enabled: %lld running: %lld\n",
				i,values[i],enabled[i],running[i]);

			if (enabled[i]) {
				printf("\tScaled count: %lld (%.2lf%%, %lld/%lld)\n",
					adjusted[i],
					(double)running[i]/(double)enabled[i]*100.0,
					running[i],enabled[i]);
			}
		}
	}

	for(i=0;i<count;i++) {
		close(fd[i]);
		munmap(addr[i],page_size);
	}

	if (!quiet) printf("\n");

	long long max=0,min=1000000000000ULL,avg=0;
	int nonzero=0;

	for(i=0;i<count;i++) {
		if (adjusted[i]>max) max=adjusted[i];
		if (adjusted[i]<min) min=adjusted[i];
		avg+=adjusted[i];
		if (adjusted[i]!=0) nonzero++;
	}
	avg=avg/nonzero;

	error=display_error(avg, // avg
				max,    // hi
				min,    // low
				1000000000ULL,quiet);

	if (error_out_of_range(error)) {
		if (!quiet) printf("Error out of range!\n");
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}
