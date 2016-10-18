/* sample_branch_stack.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* An attempt to figure out the PERF_SAMPLE_BRANCH_STACK code */

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

#include "parse_record.h"

#define MMAP_DATA_SIZE 8

#define SAMPLE_FREQUENCY 100000

int sample_type=PERF_SAMPLE_BRANCH_STACK;

int read_format=
		PERF_FORMAT_GROUP |
		PERF_FORMAT_ID |
		PERF_FORMAT_TOTAL_TIME_ENABLED |
		PERF_FORMAT_TOTAL_TIME_RUNNING;


struct validate_values validate;

static struct signal_counts {
	int in,out,msg,err,pri,hup,unknown,total;
} count = {0,0,0,0,0,0,0,0};

static int fd1,fd2;

void *our_mmap;

long long prev_head=0;

int quiet;

static void our_handler(int signum,siginfo_t *oh, void *blah) {

	int ret;

	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE, 0);

	prev_head=perf_mmap_read(our_mmap,MMAP_DATA_SIZE,prev_head,
				sample_type,read_format,0,
				&validate,quiet,NULL,RAW_NONE);

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

	ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH, 1);

	(void) ret;

}


int main(int argc, char** argv) {

	int ret;
	int mmap_pages=1+MMAP_DATA_SIZE;

	struct perf_event_attr pe;

	struct sigaction sa;
	char test_string[]="Testing PERF_SAMPLE_BRANCH_STACK...";

	quiet=test_quiet();

	if (!quiet) printf("This validates PERF_SAMPLE_BRANCH_STACK sampling.\n");

	/* set up validation */
	validate.pid=getpid();
	validate.tid=mygettid();
	validate.events=2;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}

	/* Open an INSTRUCTIONS event */

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.sample_period=SAMPLE_FREQUENCY;
	pe.sample_type=sample_type;

	/* privilege sample is 0 (for matches process) or one of the
           values */
	/* in addition one of the other bits has to be set */
	pe.branch_sample_type=PERF_SAMPLE_BRANCH_PLM_ALL|
				PERF_SAMPLE_BRANCH_ANY;

	pe.read_format=read_format;
	pe.disabled=1;
	pe.pinned=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.wakeup_events=1;

	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader "
				"INSTRUCTIONS %s\n",strerror(errno));
		}
		if (errno==ENOTSUP) {
			test_skip(test_string);
		}
     		test_fail(test_string);
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	/* Open a CYCLES event */

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_CPU_CYCLES;
	pe.sample_type=PERF_SAMPLE_IP;
	pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd2=perf_event_open(&pe,0,-1,fd1,0);
	if (fd2<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	/* Set up signal on overflow */
	our_mmap=mmap(NULL, mmap_pages*getpagesize(),
			PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);

	fcntl(fd1, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd1, F_SETSIG, SIGIO);
	fcntl(fd1, F_SETOWN,getpid());

	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);

	/* Start Counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);
	if (ret<0) {
		if (!quiet) {
			fprintf(stderr,"Error with PERF_EVENT_IOC_ENABLE "
					"of group leader: %d %s\n",
					errno,strerror(errno));
     		}
		exit(1);
 	}

	/* Our test code */
	instructions_million();

	/* Disable */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);

	if (count.total==0) {
		if (!quiet) printf("No overflow events generated.\n");
      		test_fail(test_string);
   	}

	/* Free our resources */
	munmap(our_mmap,mmap_pages*getpagesize());
	close(fd2);
	close(fd1);

	test_pass(test_string);

	return 0;
}
