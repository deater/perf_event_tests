/* openmp_overflow -- Test how overflow works on openmp programs	*/

/* Up to at least Linux 4.1 there is a bug where inherited events	*/
/* Do not inherit the fasync state, meaning children will not signal	*/
/* the parent when an overflow happens					*/

/* by Vince Weaver, vincent.weaver _at_ maine.edu			*/

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>

#include <omp.h>

#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <signal.h>
#include <fcntl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#include <syscall.h>

int gettid(void) {
	return syscall( SYS_gettid );
}

static char test_string[]="Testing OpenMP overflow results...";
static int quiet=0;
static int count_total=0;

static void our_handler(int signum, siginfo_t *info, void *uc) {

        int ret;

//	int fd = info->si_fd;

	/*  Disable will disable count for *all* threads */


//        ret=ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

	if (!quiet) printf("Overflow %d in %d\n",count_total,gettid());

        count_total++;

	/* why not refresh? */
//	ret=ioctl(fd, PERF_EVENT_IOC_ENABLE, 1);

	(void) ret;

}

int main (int argc, char **argv) {

	int nthreads, num_threads=0, tid, fd, result, read_result, i;
	int errors=0;
	struct perf_event_attr pe;
	long long count;
	struct sigaction sa;

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing OpenMP overflow behavior\n\n");
		printf("Note!!! Inherit only works if the event is created\n");
		printf("before the fork happens!  OpenMP creates thread pools\n");
		printf("so if you create the event after the thread pools hae been\n");
		printf("created it may be too late and you won't get child events!\n\n");
	}


	/****************************************/
	/* Inherit				*/
	/****************************************/

	if (!quiet) {
		printf("Testing the inherit case\n");
		printf("10Million instructions, with overflow every 1 million\n");

	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.inherit=1;
	pe.sample_period=1000000;
//	pe.wakeup_events=1;
//	pe.sample_type=PERF_SAMPLE_IP;
//	pe.freq=1;
//	pe.sample_freq=4000;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		fprintf(stderr,"Error opening event %llx %s\n",
			pe.config,strerror(errno));
		test_fail(test_string);
	}

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}

	fcntl(fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd, F_SETSIG, SIGIO);

	struct f_owner_ex {
		int   type;
		pid_t pid;
	} owner;

//	owner.type=F_OWNER_PGRP;
	owner.type=F_OWNER_PID;
	owner.pid=getpid();
	fcntl(fd, F_SETOWN_EX,&owner);

	ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

	/* Start a parallel group of threads */
#pragma omp parallel private(nthreads, i, tid)
{

	/* Obtain thread number */
	tid = omp_get_thread_num();

	/* Only master thread does this */
	if (tid == 0) {
		nthreads = omp_get_num_threads();
		num_threads = nthreads;
		if (!quiet) {
			printf("\tRunning with %d threads\n", nthreads);
		}
	}

	if (!quiet) {
		printf("\t+ Running 10 million instructions in thread %d %d\n",
			tid,gettid());
	}

	for(i=0;i<10;i++) {
		result=instructions_million();
	}

}
	/* All threads join master thread and disband */

	ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
	read_result=read(fd,&count,sizeof(long long));

	if (result==CODE_UNIMPLEMENTED) {
		fprintf(stderr,"\tCode unimplemented\n");
		test_fail(test_string);
	}

	if (read_result!=sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tCount=%lldM, expected roughly %dM\n",
			count/1000000, 10*num_threads);
		printf("\tOverflows=%d, expected roughly 100\n",count_total);
	}

	/* Should be closer to 100 */
	/* but we might be overflowing so fast some are getting lost */
	if ((count_total<50) || (count_total>200)) {
		if (!quiet) {
			printf("ERROR: unexpected overflow count! ");
			printf("Likely a kernel bug if < 4.2\n");
		}
		errors++;
	}

	close(fd);

	/************************************************/
	/* With non-inherit				*/
	/************************************************/

	if (!quiet) {
		printf("\nTesting the non-inherit case\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.inherit=1;
	pe.sample_period=1000000;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		fprintf(stderr,"Error opening event %llx %s\n",
			pe.config,strerror(errno));
		test_fail(test_string);
	}

	count_total=0;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}

	fcntl(fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd, F_SETSIG, SIGIO);

//	owner.type=F_OWNER_PGRP;
	owner.type=F_OWNER_PID;
	owner.pid=getpid();
	fcntl(fd, F_SETOWN_EX,&owner);



	ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

	/* Start a parallel group of threads */
#pragma omp parallel private(nthreads, tid, i)
{

	/* Obtain thread number */
	tid = omp_get_thread_num();

	/* Only master thread does this */
	if (tid == 0) {
		nthreads = omp_get_num_threads();
		if (!quiet) {
			printf("Running with %d threads\n", nthreads);
		}
	}

	if (!quiet) {
		printf("\t+ Running 10 million instructions in thread %d\n", 
			tid);
	}

	for(i=0;i<10;i++) {
		result=instructions_million();
	}


}
	/* All threads join master thread and disband */

	ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
	read_result=read(fd,&count,sizeof(long long));

	if (result==CODE_UNIMPLEMENTED) {
		fprintf(stderr,"\tCode unimplemented\n");
		test_fail(test_string);
	}

	if (read_result!=sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tCount=%lldM, expected roughly 10M "
			"(only counted thread 0)\n",count/1000000);
		printf("\tOverflows=%d, expected roughly 10\n",count_total);
	}
	close(fd);

	if ((count_total<10) || (count_total>20)) {
		if (!quiet) {
			printf("ERROR: unexpected overflow count!\n");
		}
		errors++;
	}

	/* Report any errors as failure */

	if (errors==0) {
		test_pass(test_string);
	}
	else {
		test_fail(test_string);
	}

	return 0;
}

