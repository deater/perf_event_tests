/* ioctl_period.c					*/
/* Test the PERF_EVENT_IOC_PERIOD ioctl			*/

/* by Vince Weaver   vincent.weaver _at_ maine.edu	*/

/* Linux 3.14 updated all architectures to match 	*/
/*	the ARM behavior.				*/

/* Since 3.7 (3581fe0ef37) ARM behaves differently	*/
/*	and updates the period immediately rather	*/
/*	than after the next overflow.			*/

/* This ioctl was broken until 2.6.36			*/
/*	ad0cf3478de8677f720ee06393b3147819568d6a	*/
/*	and always returned EFAULT			*/


#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>

#include <errno.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"
#include "instructions_testcode.h"

static char test_string[]="Testing ioctl(PERF_EVENT_IOC_PERIOD)...";

#define OVERFLOWS_TO_TRACK	1024
static long long overflow_counts[OVERFLOWS_TO_TRACK];
static int overflows=0;
static int ioctl_errors=0;
static int ioctl_errno=0;

static void our_handler(int signum, siginfo_t *info, void *uc) {

	int fd = info->si_fd;
	int ret;
	long long value;
	int read_result;
	long long new_period;

	/* Turn off measurement */
	ioctl(fd,PERF_EVENT_IOC_DISABLE,0);

	/* Read out value */
	read_result=read(fd,&value,sizeof(long long));

	overflow_counts[overflows]=value;

	/* Increment, but make sure we don't overflow */
	overflows++;
	if (overflows>=OVERFLOWS_TO_TRACK) {
		overflows=OVERFLOWS_TO_TRACK-1;
	}

	if (overflows==5) {
		new_period=100000;
		ret=ioctl(fd, PERF_EVENT_IOC_PERIOD,&new_period);
		if (ret<0) {
			ioctl_errors++;
			ioctl_errno=errno;
		}
	}

	/* Restart for one more sample period */
	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH,1);

	(void)read_result;
}


int main(int argc, char** argv) {

	int fd[2],i;
	int quiet;
	int validation_errors=0;
	int old_behavior=1;

	long long diff;

	struct perf_event_attr pe;

	struct sigaction sa;

	quiet=test_quiet();

	if (!quiet) {
		printf("This tests the PERF_EVENT_IOC_PERIOD ioctl.\n\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.sample_period=10000;

	arch_adjust_domain(&pe, quiet);

	fd[0]=perf_event_open(&pe,0,-1,-1,0);
	if (fd[0]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	/* Set up overflow */
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;
	if (sigaction( SIGRTMIN+2, &sa, NULL) < 0) {
		printf("Error setting up signal handler\n");
	}
	fcntl(fd[0], F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd[0], F_SETSIG, SIGRTMIN+2);
	fcntl(fd[0], F_SETOWN,getpid());

	ioctl(fd[0], PERF_EVENT_IOC_REFRESH,1);

	instructions_million();

	ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	close(fd[0]);

	if (!quiet) {
		printf("Overflows:\n");
		for(i=0;i<overflows;i++) {
			printf("\t%d %lld\n",i,overflow_counts[i]);
		}
	}

	if (ioctl_errors) {

		if (!quiet) {
			if (ioctl_errno==EFAULT) {
				fprintf(stderr,"Known issue with kernels <2.6.36, PERF_IOC_PERIOD always fails due to kernel bug.\n");
			}
			else {
				fprintf(stderr,"Unknown failure with PERF_EVENT_IOC_PERIOD: %s\n",strerror(errno));
			}
		}

		test_fail(test_string);
	}

	/* validate results */
	/* should be 10k apart for 0,1,2,3,4	*/

	for(i=0;i<4;i++) {
		diff=overflow_counts[i+1]-overflow_counts[i];

		if ((diff>11000) || (diff<9000)) {
			if (!quiet) {
				fprintf(stderr,"Overflow %i-%i should have been 10,000, was %lld\n",i,i+1,diff);
			}
			validation_errors++;
		}
	}

	/* 4-5 should be 10k (old behavior) or 100k (new behavior) */

	diff=overflow_counts[5]-overflow_counts[4];

	if ((diff<11000) && (diff>9000)) {
		if (!quiet) {
			fprintf(stderr,"Overflow %i-%i near 10,000 (%lld), old behavior\n",i,i+1,diff);
		}
		old_behavior=1;
	}
	else if ((diff<101000) && (diff>99000)) {
		if (!quiet) {
			fprintf(stderr,"Overflow %i-%i near 100,000 (%lld), new behavior\n",i,i+1,diff);
		}
		old_behavior=0;
	}
	else {
		if (!quiet) {
			fprintf(stderr,"Overflow %i-%i %lld, unexpected\n",i,i+1,diff);
		}
		validation_errors++;
	}


	/* 5-6 and after should be 100k		*/
	for(i=5;i<overflows-1;i++) {
		diff=overflow_counts[i+1]-overflow_counts[i];

		if ((diff>101000) || (diff<99000)) {
			if (!quiet) {
				fprintf(stderr,"Overflow %i-%i should have been 100,000, was %lld\n",i,i+1,diff);
			}
			validation_errors++;
		}
	}

	/* FIXME: also check for case where we reset
		overflow on running counter?
	*/

	if (validation_errors) {
		test_fail(test_string);
	}

	if (old_behavior) {
		test_yellow_old_behavior(test_string);
	}

	test_pass(test_string);

	return 0;
}
