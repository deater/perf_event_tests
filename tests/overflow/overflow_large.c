/* overflow_large.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* Prior to Linux 3.15 behavior was buggy with very large sample_period  */
/* Values greater than 2^63 would be cast as unsigned, and quickly       */
/* get wrapped around to a value of "2"                                  */

/* This was noticed while using perf_fuzzer                              */

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

static int fd1;

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

	if (count.total>50000) {
		/* abort!  too many! */
	}
	else {
		ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH,1);
	}

	(void) ret;

}

int main(int argc, char** argv) {
	int ret,quiet,silent;

	struct perf_event_attr pe;

	struct sigaction sa;
	void *our_mmap;
	char test_string[]="Testing large sample_period...";

	quiet=test_quiet();
	silent = quiet;  /* true prints details of error */

	if (!quiet) {
		printf("This tests behavior of large sample_period.\n");
		printf("This was broken prior to Linux 3.15.\n\n");
	}

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}

	/*******************************************************/
	/* First try sample period that *will* cause overflows */
	/*******************************************************/

	count.in=0; count.out=0; count.msg=0;
	count.err=0; count.pri=0; count.hup=0;
	count.unknown=0; count.total=0;

	if (!quiet) {
		printf("Trying 1 million instructions with period 100,000\n");
		printf("Should be 10 overflows\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.sample_period=100000;
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
		if (!silent) fprintf(stderr,"Error opening leader %llx\n",pe.config);
		test_fail(test_string);
	}

	our_mmap=mmap(NULL, (1+MMAP_PAGES)*getpagesize(),
			PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);

	fcntl(fd1, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd1, F_SETSIG, SIGIO);
	fcntl(fd1, F_SETOWN,getpid());

	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);

	ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH,1);

	if (ret<0) {
		if (!silent) fprintf(stderr,"Error with PERF_EVENT_IOC_ENABLE "
				     "of group leader: %d %s\n",errno,strerror(errno));
		test_fail(test_string);
	}

	instructions_million();

	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	close(fd1);

	if (!quiet) {
		printf("Counts, using mmap buffer %p\n",our_mmap);
		printf("\tPOLL_IN : %d\n",count.in);
		printf("\tPOLL_OUT: %d\n",count.out);
		printf("\tPOLL_MSG: %d\n",count.msg);
		printf("\tPOLL_ERR: %d\n",count.err);
		printf("\tPOLL_PRI: %d\n",count.pri);
		printf("\tPOLL_HUP: %d\n",count.hup);
		printf("\tUNKNOWN : %d\n",count.unknown);
	}

	if (count.total==0) {
		if (!silent) printf("No overflow events generated.\n");
		test_fail(test_string);
	}

	if (count.in!=0) {
		if (!silent) printf("Unexpected POLL_IN interrupt.\n");
		test_fail(test_string);
	}

	if (count.hup!=10) {
		if (!silent) printf("POLL_HUP value %d, expected %d.\n",
					count.hup,10);
		test_fail(test_string);
	}

	/*******************************************************/
	/* Next try moderate size value, should be no overflow */
	/*******************************************************/

	count.in=0; count.out=0; count.msg=0;
	count.err=0; count.pri=0; count.hup=0;
	count.unknown=0; count.total=0;

	if (!quiet) {
		printf("Trying 1 million instructions with period 10,000,000\n");
		printf("Should be 0 overflows\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.sample_period=10000000;
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
		if (!silent) fprintf(stderr,"Error opening leader %llx\n",pe.config);
		test_fail(test_string);
	}

	our_mmap=mmap(NULL, (1+MMAP_PAGES)*getpagesize(),
			PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);

	fcntl(fd1, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd1, F_SETSIG, SIGIO);
	fcntl(fd1, F_SETOWN,getpid());

	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);

	ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH,1);

	if (ret<0) {
		if (!silent) fprintf(stderr,"Error with PERF_EVENT_IOC_ENABLE of "
				     "group leader: %d %s\n",errno,strerror(errno));
		test_fail(test_string);
	}

	instructions_million();

	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	close(fd1);

	if (!quiet) {
		printf("Counts, using mmap buffer %p\n",our_mmap);
		printf("\tPOLL_IN : %d\n",count.in);
		printf("\tPOLL_OUT: %d\n",count.out);
		printf("\tPOLL_MSG: %d\n",count.msg);
		printf("\tPOLL_ERR: %d\n",count.err);
		printf("\tPOLL_PRI: %d\n",count.pri);
		printf("\tPOLL_HUP: %d\n",count.hup);
		printf("\tUNKNOWN : %d\n",count.unknown);
	}

	if (count.total!=0) {
		if (!silent) printf("Unexpected overflow events generated.\n");
		test_fail(test_string);
	}


	/*******************************************************/
	/* Next try large size value, should be no overflow    */
	/*******************************************************/

	count.in=0; count.out=0; count.msg=0;
	count.err=0; count.pri=0; count.hup=0;
	count.unknown=0; count.total=0;

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.sample_period=(1ULL<<63)-1;
	pe.sample_type=PERF_SAMPLE_IP;
	pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
	pe.disabled=1;
	pe.pinned=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.wakeup_events=1;

	if (!quiet) {
		printf("Trying 1 million instructions with period (2^63)-1 (%llx)\n",
			pe.sample_period);
		printf("Should be 0 overflows\n");
	}

	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!silent) fprintf(stderr,"Error opening leader %llx\n",pe.config);
		test_fail(test_string);
	}

	our_mmap=mmap(NULL, (1+MMAP_PAGES)*getpagesize(),
			PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);

	fcntl(fd1, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd1, F_SETSIG, SIGIO);
	fcntl(fd1, F_SETOWN,getpid());

	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);

	ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH,1);
	if (ret<0) {
		if (!silent) fprintf(stderr,"Error with PERF_EVENT_IOC_ENABLE "
				     "of group leader: %d %s\n",errno,strerror(errno));
		test_fail(test_string);
	}

	instructions_million();

	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	close(fd1);

	if (!quiet) {
		printf("Counts, using mmap buffer %p\n",our_mmap);
		printf("\tPOLL_IN : %d\n",count.in);
		printf("\tPOLL_OUT: %d\n",count.out);
		printf("\tPOLL_MSG: %d\n",count.msg);
		printf("\tPOLL_ERR: %d\n",count.err);
		printf("\tPOLL_PRI: %d\n",count.pri);
		printf("\tPOLL_HUP: %d\n",count.hup);
		printf("\tUNKNOWN : %d\n",count.unknown);
	}

	if (count.total!=0) {
		if (!silent) printf("Unexpected overflow events generated.\n");
		test_fail(test_string);
	}

	/************************************************************/
	/* Next try very large size value, should be no overflow    */
	/* In fact, on 3.15 and later the open should fail          */
	/************************************************************/

	count.in=0; count.out=0; count.msg=0;
	count.err=0; count.pri=0; count.hup=0;
	count.unknown=0; count.total=0;

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.sample_period=0xc0000000000000bd;
	pe.sample_type=PERF_SAMPLE_IP;
	pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
	pe.disabled=1;
	pe.pinned=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.wakeup_events=1;

	if (!quiet) {
		printf("Trying 1 million instructions with period %llx\n",
			pe.sample_period);
		printf("Should be 0 overflows\n");
	}

	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (errno==EINVAL) {
			if (!quiet) {
				fprintf(stderr,"Properly failed with too-large sample_period\n");
			}
			test_pass(test_string);
			exit(0);
		}
		if (!silent) fprintf(stderr,"Error opening leader %llx\n",pe.config);
		test_fail(test_string);
	}

	our_mmap=mmap(NULL, (1+MMAP_PAGES)*getpagesize(),
			PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);

	fcntl(fd1, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd1, F_SETSIG, SIGIO);
	fcntl(fd1, F_SETOWN,getpid());

	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);

	ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH,1);

	if (ret<0) {
		if (!silent) {
			fprintf(stderr,"Error with PERF_EVENT_IOC_ENABLE of group leader: "
					"%d %s\n",errno,strerror(errno));
		}
		test_fail(test_string);
	}

	instructions_million();

	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	close(fd1);

	if (!quiet) {
		printf("Counts, using mmap buffer %p\n",our_mmap);
		printf("\tPOLL_IN : %d\n",count.in);
		printf("\tPOLL_OUT: %d\n",count.out);
		printf("\tPOLL_MSG: %d\n",count.msg);
		printf("\tPOLL_ERR: %d\n",count.err);
		printf("\tPOLL_PRI: %d\n",count.pri);
		printf("\tPOLL_HUP: %d\n",count.hup);
		printf("\tUNKNOWN : %d\n",count.unknown);
	}

	if (count.total>20000) {
		int version;
		if (!silent) printf("Stopping early, too many overflows encountered.\n");
		/* This is expected before 3.15 */
		version=get_kernel_version();
		if (version<0x30f00) {
			test_fail_kernel(test_string);
		}
		else {
			test_fail(test_string);
		}
	}

	if (count.total!=0) {
		if (!silent) printf("Unexpected overflow events generated.\n");
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}
