/* simultaneous_overflow.c  */
/* by Vince Weaver   vincent.weaver@maine.edu */

/* Test to see if we can sample on two events at once */

/* A user was having a PAPI issue related to this */

/* This test started failing in 4.12-rc4 */
/* cc1582c231ea041fbc68861dfaf957eaf902b829 */
/* The "security" fix that dropped user samples with kernel addresses */
/* It was finally reverted in 6a8a75f3235724c5941a33e287b2f98966ad14c5 */
/* in 4.13-rc2 */

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

#define MMAP_PAGES 16

#define NUM_EVENTS 2

struct {
	int fd;
	int overflows;
	int individual_overflow;
} events[NUM_EVENTS];

#define SNB_TEST 0

#if SNB_TEST
struct {
	char name[BUFSIZ];
	int type;
	int config;
	int period;
} event_values[NUM_EVENTS] = {
	{ "PAPI_TOT_CYC", PERF_TYPE_RAW, 0x53003c, 4000000},
	{ "PAPI_L2_TCM", PERF_TYPE_RAW, 0x53022e, 400000},
};
#else
struct {
	char name[BUFSIZ];
	int type;
	int config;
	int period;
} event_values[NUM_EVENTS] = {
	{ "perf::instructions",
		PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, 1000000},
	{ "perf::instructions",
		PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, 2000000},
};
#endif

/* Count the events */
/* Don't bother disabling/refreshing? */
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
	void *our_mmap[NUM_EVENTS];
	char test_string[]="Testing multiple event overflow...";

	long long counts[NUM_EVENTS];

	for(i=0;i<NUM_EVENTS;i++) {
		events[i].fd=-1;
		events[i].overflows=0;
	}

	quiet=test_quiet();

	/*********************************************************************/
	if (!quiet) printf("This tests simultaneous overflow.\n");
	/*********************************************************************/

	/*****************************/
	/* set up our signal handler */
	/*****************************/

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	/* Use Real Time signal, because it is queued                */
	/* If we just use SIGIO then we lose signals if one comes in */
	/* while we were handling a previous overflow.               */

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
		pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
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

		our_mmap[i]=mmap(NULL, (1+MMAP_PAGES)*getpagesize(),
			PROT_READ|PROT_WRITE, MAP_SHARED, events[i].fd, 0);

		fcntl(events[i].fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
		fcntl(events[i].fd, F_SETSIG, SIGRTMIN+2);
		fcntl(events[i].fd, F_SETOWN,getpid());

		ioctl(events[i].fd, PERF_EVENT_IOC_RESET, 0);

		if (!quiet) {
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
		//xpe.sample_type=PERF_SAMPLE_ID;
		// pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
		pe.disabled=1;
		pe.pinned=1;
		pe.exclude_kernel=1;
		pe.exclude_hv=1;
		pe.wakeup_events=1;

		arch_adjust_domain(&pe,quiet);

		events[i].fd=perf_event_open(&pe,0,-1,-1,0);
		if (events[i].fd<0) {
			fprintf(stderr,"Error opening %llx\n",pe.config);
			test_fail(test_string);
		}

		/* on older kernels you need this even if you don't use it */
		our_mmap[i]=mmap(NULL, (1+MMAP_PAGES)*getpagesize(),
				PROT_READ|PROT_WRITE, MAP_SHARED,
				events[i].fd, 0);

		fcntl(events[i].fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
		fcntl(events[i].fd, F_SETSIG, SIGRTMIN+2);
		fcntl(events[i].fd, F_SETOWN,getpid());

		ioctl(events[i].fd, PERF_EVENT_IOC_RESET, 0);
	}

	for(i=0;i<NUM_EVENTS;i++) {
		ret=ioctl(events[i].fd, PERF_EVENT_IOC_ENABLE,0);
	}

	if (ret<0) {
		if (!quiet) {
			printf("Error with PERF_EVENT_IOC_ENABLE of "
				"group leader: %d %s\n",
				errno,strerror(errno));
		}
		test_fail(test_string);
	}

	if (!quiet) printf("\t");
	naive_matrix_multiply(quiet);

	for(i=0;i<NUM_EVENTS;i++) {
		ret=ioctl(events[i].fd, PERF_EVENT_IOC_DISABLE,0);
	}

	for(i=0;i<NUM_EVENTS;i++) {
		if (!quiet) {
			printf("\tfd %d overflows: %d (%s/%d)\n",
				events[i].fd,events[i].overflows,
				event_values[i].name,event_values[i].period);
		}
	}

	for(i=0;i<NUM_EVENTS;i++) {
		if (events[i].overflows==0) {
			if (!quiet) printf("No overflow events generated.\n");
			test_fail(test_string);
		}
	}

	for(i=0;i<NUM_EVENTS;i++) {
		read(events[i].fd,&counts[i],8);
		if (!quiet) printf("\tRead %d %lld\n",events[i].fd,counts[i]);
	}

	for(i=0;i<NUM_EVENTS;i++) {
		close(events[i].fd);
	}

	/* test validity */
	for(i=0;i<NUM_EVENTS;i++) {

		if (!quiet) {
			printf("Event %s/%d Expected %lld Got %d\n",
				event_values[i].name, event_values[i].period,
				counts[i]/event_values[i].period,events[i].overflows);
		}
		if (counts[i]/event_values[i].period!=events[i].overflows) {
		}
		else {
			matches++;
		}
	}

	if (matches!=NUM_EVENTS) {
		fprintf(stderr,"Wrong number of overflows!\n");
		test_fail(test_string);
	}

	test_pass(test_string);

	(void)our_mmap;

	return 0;
}

