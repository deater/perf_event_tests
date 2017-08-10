/* overflow_requires_mmap.c  */

/* On older kernels you could not get overflow signals	*/
/* on an event without having a mmap() buffer mapped	*/

/* This was fixed by f506b3dc0ec454a16d40cab9ee5d75435b39dc50 in Linux 3.0 */

/* by Vince Weaver --- vincent.weaver@maine.edu */

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

static int count=0;

static void our_handler(int signum,siginfo_t *oh, void *blah) {
	count++;
}

double busywork(int count) {

	int i;
	double sum=0.0012;

	for(i=0;i<count;i++) {
		sum+=0.01;
	}
	return sum;
}


int main(int argc, char** argv) {

	int ret,fd1,fd2,quiet;
	double result;

	struct perf_event_attr pe;

	struct sigaction sa;

	char test_string[]="Testing if overflow signals require ring buffer...";

	quiet=test_quiet();

	if (!quiet) {
		printf("Check if you can obtain overflow signals on events\n");
		printf("without an associated mmap'd ring buffer\n");
	}

	/* set up signal handler */
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sa, NULL) < 0) {
		printf("Error setting up signal handler\n");
		test_fail(test_string);
	}

	/* set up perf event 1 */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
	pe.disabled=1;
	pe.pinned=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			printf("Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	/* setup event 2, a sampling event */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.sample_period=1000000;
	pe.sample_type=PERF_SAMPLE_IP;
	pe.read_format=0;
	pe.disabled=0;
	pe.pinned=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.wakeup_events=1;

	arch_adjust_domain(&pe,quiet);

	fd2=perf_event_open(&pe,0,-1,fd1,0);
	if (fd2<0) {
		if (!quiet) {
			printf("Error opening %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	/* On old kernels you get no overflows w/o this mmap */
#if 0
	{
	void *blargh;

	blargh=mmap(NULL, (1+1)*4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);
	}
#endif

	/* setup event 2 to have overflow signals */
	fcntl(fd2, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd2, F_SETSIG, SIGIO);
	fcntl(fd2, F_SETOWN,getpid());

	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd2, PERF_EVENT_IOC_RESET, 0);

	/* enable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);

	result=busywork(10000000);

	/* disable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Disable failure\n");
	}

	/* need to consume some of these values or compiler optimizes away */
	if (!quiet) printf("Count: %d %lf\n",count,result);

	/* if overflows, then we pass */
	if (count>0) {
		test_pass(test_string);
	}
	else {
		test_yellow_old_behavior(test_string);
	}

	return 0;
}

