/* signal_after_close.c  */
/* Can we still get signals after close of a fd? */
/* The answer is yes, if event is still mmap'd */

/* by Vince Weaver   vincent.weaver@maine.edu */

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


static char test_string[]="Testing signal after close...";
static int count1=0,count2=0;
static int fd1,fd2;

static void our_handler2(int signum,siginfo_t *oh, void *blah) {

	int ret;

	count2++;

//	printf("Handler2: count2=%d fd=%d\n",count2,oh->si_fd);

	ret=ioctl(oh->si_fd, PERF_EVENT_IOC_REFRESH,1);

	// Normal to get error, as fd==4 has been closed
//	if (ret<0) {
//		printf("Errror fd=%d!\n",oh->si_fd);
//	}
	(void)ret;
}

static void our_handler(int signum,siginfo_t *oh, void *blah) {
	count1++;
	ioctl(oh->si_fd, PERF_EVENT_IOC_REFRESH,1);
}

int main(int argc, char** argv) {

	int quiet,i;
	int ret;
	double result;
	void *blargh;
	struct perf_event_attr pe;

	struct sigaction sa;

	quiet=test_quiet();

	if (!quiet) {
		printf("\nThis looks at signal handling after close\n");
		printf("As long as you don't munmap() the mmap buffer\n");
		printf("A closed event can still generate overflows\n\n");
	}

	/* setup signal handler */
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}

	/* setup first event, which samples */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	//pe.sample_period=1000000;
	pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
//	pe.sample_type=PERF_SAMPLE_IP;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.wakeup_events=1;
	pe.disabled=1;

	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		printf("Error opening %llx\n",pe.config);
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


	blargh=mmap(NULL, (1+1)*4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);

	/* setup our fd to signal on overflow */
	fcntl(fd2, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd2, F_SETSIG, SIGIO);
	fcntl(fd2, F_SETOWN,getpid());

	/* start counting */

	ret=ioctl(fd2, PERF_EVENT_IOC_REFRESH,0);

	result=instructions_million();

	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);

	if (!quiet) {
		printf("Count1: %d %p\n",count1,blargh);
	}


	close(fd1);
	close(fd2);

	/* Test 2, 0 refresh */

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

//	blargh2=mmap(NULL, (1+1)*4096,
//		PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);

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

	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);

	if (!quiet) printf("Count2: %d\n",count2);

	if (count2==1) {
		if (!quiet) fprintf(stderr,"Only counted one overflow.\n");
		test_fail(test_string);
	}
	else if (count2==0) {
		if (!quiet) printf("No overflow events generated.\n");
		test_fail(test_string);
	}
	else if ((count2>900) && (count2<1200)) {
		if (!quiet) printf("Closed fd unexpectedly did *not* overflow.\n");
		test_fail(test_string);
	}
	else if ((count2>1900) && (count2<2200)) {
		test_pass(test_string);
	}
	else {
		if (!quiet) printf("Weird overflow count.\n");
		test_fail(test_string);
	}
	return 0;
}

