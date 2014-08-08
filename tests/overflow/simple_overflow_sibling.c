/* simple_overflow_sibling.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* Test overflow of sibling */

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
#include "instructions_testcode.h"

#define MMAP_PAGES 8

static struct signal_counts {
  int in,out,msg,err,pri,hup,unknown,total;
} count = {0,0,0,0,0,0,0,0};


static int fd1,fd2;

static void our_handler(int signum,siginfo_t *oh, void *blah) {

 int ret;

  ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE, 0);

  switch(oh->si_code) {
     case POLL_IN:  count.in++;  break;
     case POLL_OUT: count.out++; break;
     case POLL_MSG: count.msg++; break;
     case POLL_ERR: count.err++; break;
     case POLL_PRI: count.pri++; break;
     case POLL_HUP: count.hup++; break;
     default: count.unknown++; break;
  }

  count.total++;

  ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,1);

  (void) ret;

}


int main(int argc, char** argv) {

	int ret,quiet,i;

	struct perf_event_attr pe;

	struct sigaction sa;
	void *our_mmap;
	char test_string[]="Testing overflows on sibling...";

	quiet=test_quiet();

	if (!quiet) printf("This tests that overflows of siblings work.\n");

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_CPU_CYCLES;
	pe.sample_period=0;
	pe.sample_type=0;
	pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
	pe.disabled=1;
	pe.pinned=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.wakeup_events=1;

	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		fprintf(stderr,"Error opening leader %llx\n",pe.config);
		test_fail(test_string);
	}

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.sample_period=100000;
	pe.sample_type=PERF_SAMPLE_IP;
	pe.read_format=0;
	pe.disabled=0;
	pe.pinned=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd2=perf_event_open(&pe,0,-1,fd1,0);
	if (fd2<0) {
		fprintf(stderr,"Error opening %llx\n",pe.config);
			test_fail(test_string);
	}

	/* large enough that threshold not a problem */
	our_mmap=mmap(NULL, (1+MMAP_PAGES)*getpagesize(),
		PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);

	fcntl(fd2, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd2, F_SETSIG, SIGIO);
	fcntl(fd2, F_SETOWN,getpid());

	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd2, PERF_EVENT_IOC_RESET, 0);

	ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);

	if (ret<0) {
		if (!quiet) fprintf(stderr,"Error with PERF_EVENT_IOC_ENABLE of group leader: "
			"%d %s\n",errno,strerror(errno));
		test_fail(test_string);
	}

	for(i=0;i<100;i++) {
		instructions_million();
	}

	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);

	if (!quiet) printf("Count: %d %p\n",count.total,our_mmap);

	if (count.total==0) {
		if (!quiet) printf("No overflow events generated.\n");
		test_fail(test_string);
	}

	if (count.total!=1000) {
		if (!quiet) printf("Expected %d overflows, got %d.\n",
				count.total,100);
		test_fail(test_string);
	}

	close(fd1);
	close(fd2);

	test_pass(test_string);

	return 0;
}

