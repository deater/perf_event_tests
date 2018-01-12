/* simultaneous_group_overflow.c              */
/* by Vince Weaver   vincent.weaver@maine.edu */

/* Test to see if we can sample on two events at once */
/* within the same group.                             */

/* This test can fail on ivybridge(?) but not haswell or skylake? */

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

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "matrix_multiply.h"

#define MMAP_PAGES 8

#define NUM_EVENTS 2

struct {
	int fd;
	int overflows;
	int individual_overflow;
} events[NUM_EVENTS];

struct {
	char name[BUFSIZ];
	int type;
	int config;
	int period;
} event_values[NUM_EVENTS] = {
	{ "perf::instructions", PERF_TYPE_HARDWARE,
		PERF_COUNT_HW_INSTRUCTIONS, 1000000},
	{ "perf::instructions", PERF_TYPE_HARDWARE,
		PERF_COUNT_HW_INSTRUCTIONS, 2000000},
};



static void our_handler(int signum,siginfo_t *info, void *uc) {

	int ret,i;
	int fd = info->si_fd;

	//  ret=ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

	for(i=0;i<NUM_EVENTS;i++) {
		if (events[i].fd==fd) {
			events[i].overflows++;
			break;
		}
	}

	if (i==NUM_EVENTS) printf("fd %d not found\n",fd);

	//printf("fd: %d overflowed\n",fd);
	//ret=ioctl(fd, PERF_EVENT_IOC_REFRESH,1);

	(void) ret;

}



int main(int argc, char** argv) {

	int ret,quiet,i,matches=0;

	struct perf_event_attr pe;

	struct sigaction sa;
	void *our_mmap;
	char test_string[]="Testing multiple event overflow within group...";

	for(i=0;i<NUM_EVENTS;i++) {
		events[i].fd=-1;
		events[i].overflows=0;
	}

	quiet=test_quiet();

	/*********************************************************************/
	if (!quiet) printf("This tests simultaneous overflow within group.\n");
	/*********************************************************************/

	/*****************************/
	/* set up our signal handler */
	/*****************************/

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGRTMIN+2, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}

	/***********************/
	/* get expected counts */
	/***********************/

	for(i=0;i<NUM_EVENTS;i++) {
		memset(&pe,0,sizeof(struct perf_event_attr));

		pe.type=event_values[i].type;
		pe.size=sizeof(struct perf_event_attr);
		pe.config=event_values[i].config;
		pe.sample_period=event_values[i].period;
		pe.sample_type=0;
		pe.read_format=PERF_FORMAT_GROUP;
		pe.disabled=1;
		pe.pinned=0;
		pe.exclude_kernel=1;
		pe.exclude_hv=1;
		pe.wakeup_events=1;

		arch_adjust_domain(&pe,quiet);

		events[i].fd=perf_event_open(&pe,0,-1,-1,0);
		if (events[i].fd<0) {
		fprintf(stderr,"Error opening leader %llx\n",pe.config);
		test_fail(test_string);
	}

	/* on older kernels you need this even if you don't use it */
	our_mmap=mmap(NULL, (1+MMAP_PAGES)*getpagesize(),
		PROT_READ|PROT_WRITE, MAP_SHARED, events[i].fd, 0);

	fcntl(events[i].fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(events[i].fd, F_SETSIG, SIGRTMIN+2);
	fcntl(events[i].fd, F_SETOWN,getpid());

	ioctl(events[i].fd, PERF_EVENT_IOC_RESET, 0);

	if (!quiet) {
		printf("Testing matrix matrix multiply\n");
		printf("\tEvent %s with period %d\n",
			event_values[i].name,
			event_values[i].period);
	}

	ret=ioctl(events[i].fd, PERF_EVENT_IOC_ENABLE,0);

	if (!quiet) printf("\t");
	naive_matrix_multiply(quiet);

	ret=ioctl(events[i].fd, PERF_EVENT_IOC_DISABLE,0);

      if (!quiet) {
         printf("\tfd %d overflows: %d (%s/%d)\n",
		events[i].fd,events[i].overflows,
	        event_values[i].name,
		event_values[i].period);
	}
	if (events[i].overflows==0) {
		if (!quiet) printf("No overflow events generated.\n");
			test_fail(test_string);
		}

		close(events[i].fd);

		events[i].individual_overflow=events[i].overflows;

		events[i].overflows=0;
		events[i].fd=-1;
	}


	/**********************************/
	/* test overflow for both         */
	/**********************************/

	if (!quiet) {
		printf("Testing matrix matrix multiply\n");
		for(i=0;i<NUM_EVENTS;i++) {
			printf("\tEvent %s with period %d\n",
				event_values[i].name,
				event_values[i].period);
			}
	}

	for(i=0;i<NUM_EVENTS;i++) {

		memset(&pe,0,sizeof(struct perf_event_attr));

		pe.type=event_values[i].type;
		pe.size=sizeof(struct perf_event_attr);
		pe.config=event_values[i].config;
		pe.sample_period=event_values[i].period;
		pe.sample_type=0;
		pe.read_format=PERF_FORMAT_GROUP;
		pe.disabled=(i==0)?1:0;
		pe.pinned=(i==0)?1:0;
		pe.exclude_kernel=1;
		pe.exclude_hv=1;
		pe.wakeup_events=1;

		arch_adjust_domain(&pe,quiet);

		events[i].fd=perf_event_open(&pe,0,-1,(i==0)?-1:events[0].fd,0);
		if (events[i].fd<0) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
			test_fail(test_string);
		}

		/* on older kernels you need this even if you don't use it */
		our_mmap=mmap(NULL, (1+MMAP_PAGES)*getpagesize(),
			PROT_READ|PROT_WRITE, MAP_SHARED, events[i].fd, 0);

		fcntl(events[i].fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
		fcntl(events[i].fd, F_SETSIG, SIGRTMIN+2);
		fcntl(events[i].fd, F_SETOWN,getpid());

		ioctl(events[i].fd, PERF_EVENT_IOC_RESET, 0);
	}

	ret=ioctl(events[0].fd, PERF_EVENT_IOC_ENABLE,0);

	if (ret<0) {
		if (!quiet) {
			fprintf(stderr,"Error with PERF_EVENT_IOC_ENABLE "
				"of group leader: "
				"%d %s\n",errno,strerror(errno));
		}
		test_fail(test_string);
	}

	if (!quiet) printf("\t");
	naive_matrix_multiply(quiet);

	ret=ioctl(events[0].fd, PERF_EVENT_IOC_DISABLE,0);

	for(i=0;i<NUM_EVENTS;i++) {
		if (!quiet) {
			printf("\tfd %d overflows: %d (%s/%d)\n",
				events[i].fd,events[i].overflows,
				event_values[i].name,event_values[i].period);
		}
	}

	int result;
	long long values[1+1*NUM_EVENTS],counts[NUM_EVENTS];
	result=read(events[0].fd,&values,8*(1+1*NUM_EVENTS));

	if (result!=8*(1+1*NUM_EVENTS)) {
		if (!quiet) printf("error reading\n");
		test_fail(test_string);
	}

	if (values[0]!=NUM_EVENTS) {
		if (!quiet) printf("error reading\n");
		test_fail(test_string);
	}
	for(i=0;i<NUM_EVENTS;i++) {
		counts[i]=values[i+1];
		if (!quiet) printf("\tCount %d: %lld\n",i,counts[i]);
	}

	for(i=0;i<NUM_EVENTS;i++) {
		if (events[i].overflows==0) {
			if (!quiet) printf("No overflow events generated.\n");
			test_fail(test_string);
		}
	}

	for(i=0;i<NUM_EVENTS;i++) {
		close(events[i].fd);
	}


	/* test validity */
	for(i=0;i<NUM_EVENTS;i++) {

		if (!quiet) {
			fprintf(stderr,"Event %s/%d Expected %lld Got %d\n",
				event_values[i].name, event_values[i].period,
				counts[i]/event_values[i].period,events[i].overflows);
		}
		if (counts[i]/event_values[i].period!=events[i].overflows) {
		}
		else {
			matches++;
		}
	}

	if (counts[0]!=counts[1]) {
		if (!quiet) {
			fprintf(stderr,"Counts should be the same %lld %lld\n",
				counts[0],counts[1]);
		}
		test_fail(test_string);
	}

	if (matches!=NUM_EVENTS) {

		if (!quiet) fprintf(stderr,"Unexpected event count!\n");
		test_fail(test_string);
	}

	test_pass(test_string);

	(void)our_mmap;

	return 0;
}

