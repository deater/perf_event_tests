/* does rdpmc work on FORMAT_GROUP eventsets? */
/* what if there's a system-wide event blocking a slot? */

/* by Vince Weaver, vincent.weaver _at_ maine.edu       */


char test_string[]="Testing if rdpmc works with event groups+syswide...";
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

	long long start_before,stop_after;

	void *addr[MAX_EVENTS];

	struct perf_event_attr pe,syswide;

	int fd[MAX_EVENTS],ret1,ret2,syswide_fd[MAX_EVENTS];

	unsigned long long values[MAX_EVENTS],enabled[MAX_EVENTS],running[MAX_EVENTS];
	unsigned long long values2[MAX_EVENTS],enabled2[MAX_EVENTS],running2[MAX_EVENTS];

	double error;

	int count=3;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if rdpmc works with event groups.\n");
		printf("and if a syswide event is happening at the same time.\n\n");
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
		}
		else {
			pe.disabled=0;
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



	/* Open syswide event  */
	memset(&syswide,0,sizeof(struct perf_event_attr));
	syswide.type=PERF_TYPE_HARDWARE;
	syswide.size=sizeof(struct perf_event_attr);
	syswide.config=PERF_COUNT_HW_INSTRUCTIONS;
	syswide.disabled=0;
	for(i=0;i<4;i++) {
		syswide_fd[i]=perf_event_open(&syswide,0,-1,-1,0);
		if (syswide_fd[i]<0) {
			fprintf(stderr,"Error opening syswide event\n");
			test_fail(test_string);
		}
		ret1=ioctl(syswide_fd[i], PERF_EVENT_IOC_ENABLE,0);
	}

	/* start the rest */
	start_before=rdtsc();
	ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);
	if (ret1<0) {
		fprintf(stderr,"Error enabling!\n");
	}

	for(i=0;i<100;i++) {
		result=instructions_million();
	}

	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

	/* read */
	for(i=0;i<count;i++) {
		values[i] = mmap_read_self(addr[i], &enabled[i], &running[i]);
	}

	for(i=0;i<100;i++) {
		result=instructions_million();
	}

	/* read */
	for(i=0;i<count;i++) {
		long long rvalue;
		int result;
		result=read(fd[i],&rvalue,8);
		if (result!=8) {
			fprintf(stderr,"Read error!\n");
			exit(1);
		}
		values2[i] = rvalue;
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
			printf("\tEvent %x -- Raw count: %lld enabled: %lld running: %lld\n",
				i,values[i],enabled[i],running[i]);
			printf("\t%lld\n",values[i]*enabled[i]/running[i]);
			printf("\tEvent %x -- Raw count: %lld enabled: %lld running: %lld\n",
				i,values2[i],enabled2[i],running2[i]);
		}
	}

	for(i=0;i<count;i++) {
		close(fd[i]);
		munmap(addr[i],page_size);
	}

	if (!quiet) printf("\n");

	error=display_error(values[0], // avg
				values[0],    // hi
				values[0],    // low
				100000000ULL,quiet);

	if ((error>1.0) || ( error<-1.0)) {
		if (!quiet) printf("Error out of range!\n");
		test_fail(test_string);
	}

	error=display_error(values2[0], // avg
				values2[0],    // hi
				values2[0],    // low
				200000000ULL,quiet);

	if ((error>1.0) || ( error<-1.0)) {
		if (!quiet) printf("Error out of range!\n");
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}
