/* ioctl_2_refresh.c  */
/* by Vince Weaver   vincent.weaver@maine.edu */

/* Test out ioctl(PERF_IOC_REFRESH); */

/* TODO: out of bounds?  Negative refresh? */

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
#include "perf_helpers.h"
#include "test_utils.h"

#include "instructions_testcode.h"


static char test_string[]="Testing ioctl(PERF_IOC_REFRESH)...";
static int count=0;
static int fd1;

static void our_handler2(int signum,siginfo_t *oh, void *blah) {

	int ret;

	count++;

//	printf("COUNT2=%d %d\n",count,oh->si_fd);
	ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH,0);
	(void) ret;
}

static void our_handler(int signum,siginfo_t *oh, void *blah) {

	count++;
}

int main(int argc, char** argv) {

	int quiet,i;
	int ret,fd2;
	double result;
	void *blargh;
	struct perf_event_attr pe;

	struct sigaction sa;

	quiet=test_quiet();

	if (!quiet) {
		printf("\nThis check tests if PERF_EVENT_IOC_REFRESH on\n");
		printf("  a group leader enables sampled events in siblibgs.\n");
		printf("  This behavior was removed in 2.6.37\n");
	}

	/* setup signal handler */
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}

	/* setup first event, which has no sampling */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.wakeup_events=1;

	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		printf("Error opening leader %llx\n",pe.config);
		test_fail(test_string);
	}

	/* setup second event, which samples */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.sample_period=1000000;
	pe.sample_type=PERF_SAMPLE_IP;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.wakeup_events=1;

	arch_adjust_domain(&pe,quiet);

	fd2=perf_event_open(&pe,0,-1,fd1,0);
	if (fd2<0) {
		printf("Error opening %llx\n",pe.config);
		test_fail(test_string);
	}

	/* before at least Linux 3.0 you have to have a mmap'd	*/
	/* ring-buffer or you won't get any samples.		*/
	/* This took me 6 hours to figure out.			*/

	blargh=mmap(NULL, (1+1)*4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);

	/* setup our fd to signal on overflow */
	fcntl(fd2, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd2, F_SETSIG, SIGIO);
	fcntl(fd2, F_SETOWN,getpid());

	/* start counting */
	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd2, PERF_EVENT_IOC_RESET, 0);

	/* UNDEFINED BEHAVIOR!  This shouldn't work... */
	ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH,0);

	if (ret<0) {
		if (!quiet) {
			printf("Proper error returned with PERF_EVENT_IOC_REFRESH "
				"of group leader: %d %s\n",errno,strerror(errno));
		}

	}
	else {
		result=instructions_million();

		ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);

		/* Need to consume or compiler complains/optimizes away */
		if (!quiet) {
			printf("Count: %d %lf %p\n",count,result,blargh);
		}

		if (count>0) {
			if (!quiet) printf("Unsupported enabling of sibling events\n");
			test_yellow_old_behavior(test_string);
		}
		else {
			if (!quiet) printf("No overflow events generated.\n");
			test_fail(test_string);
		}
	}

	close(fd1);

	/* Need to munmap or can still generate signals */
	munmap(blargh, (1+1)*4096);

	close(fd2);


	/* Test 2, 0 refresh */

	count=0;

	if (!quiet) printf("\n\nThis tests if PERF_IOC_REFRESH with 0 works.\n");

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler2;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.sample_period=1000000;
	pe.sample_type=PERF_SAMPLE_IP;
	pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
	pe.disabled=1;
	pe.pinned=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.wakeup_events=1;

	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) fprintf(stderr,"Error opening leader %llx\n",pe.config);
		test_fail(test_string);
	}

	blargh=mmap(NULL, (1+1)*4096,
		PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);

	fcntl(fd1, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd1, F_SETSIG, SIGIO);
	fcntl(fd1, F_SETOWN,getpid());

	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);

	ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH, 1);

	if (ret<0) {
		if (!quiet) {
			fprintf(stderr,"Error with PERF_EVENT_IOC_REFRESH "
				"of group leader: %d %s\n",
				errno,strerror(errno));
		}
		test_fail(test_string);
	}

	for(i=0;i<1000;i++) {
		result=instructions_million();
	}

	if (result==CODE_UNIMPLEMENTED) {
		fprintf(stderr,"\tCode unimplemented\n");
		test_skip(test_string);
	}

//	naive_matrix_multiply(quiet);

	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);

	if (!quiet) printf("Count: %d %p\n",count,blargh);

	if (count==1) {
		if (!quiet) fprintf(stderr,"Only counted one overflow.\n");
		test_fail(test_string);
	}
	else if ((count>900) && (count<1100)) {
		test_pass(test_string);
	}
	else {
		if (!quiet) printf("Unexpected overflow events generated.\n");
		test_fail(test_string);
	}

	return 0;
}

