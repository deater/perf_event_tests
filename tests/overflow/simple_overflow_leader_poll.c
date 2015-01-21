/* simple_overflow_leader_poll.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* Just does some tests of the overflow infrastructure */
/*  on event group leaders.                            */

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

#include <poll.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#define MMAP_PAGES 8

static struct signal_counts {
	int in,out,msg,err,pri,hup,unknown,total;
} count = {0,0,0,0,0,0,0,0};







int main(int argc, char** argv) {

	int ret,quiet;
	int i;
	int fd1;

	struct perf_event_attr pe;

	void *our_mmap;
	char test_string[]="Testing overflow on leaders...";

	quiet=test_quiet();

	if (!quiet) printf("This tests overflow on leaders.\n");


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

	/* Not needed on 3.2? */
	pe.wakeup_events=1;

	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) fprintf(stderr,"Error opening leader %llx\n",pe.config);
		test_fail(test_string);
	}

	/* large enough that threshold not crossed */
	our_mmap=mmap(NULL, (1+MMAP_PAGES)*getpagesize(),
		PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);

	/* set up poll */
	struct pollfd fds[1];
        int result;

        fds[0].fd=fd1;
        fds[0].events=POLLIN|POLLHUP|POLLNVAL|POLLERR;

	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);
	ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE, 0);

	if (ret<0) {
		if (!quiet) {
			fprintf(stderr,"Error with PERF_EVENT_IOC_ENABLE of group leader: "
				"%d %s\n",errno,strerror(errno));
		}
		exit(1);
	}

	for (i=0;i<100;i++) {
		instructions_million();

		result=poll(fds,1,100);
		if (result==0) {
			printf("None\n");
		}
                else if (result==-1) {
			printf("Error: %s\n",strerror(errno));
			break;
		}

		if (fds[0].revents&POLLIN) count.in++;
		if (fds[0].revents&POLLHUP) count.hup++;
		if (fds[0].revents&POLLERR) count.err++;
		if (fds[0].revents&POLLOUT) count.out++;
//		case POLL_MSG: count.msg++; break;
//		case POLL_PRI: count.pri++; break;
//		default: count.unknown++; break;

	}

	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE, 0);

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

	close(fd1);

//	if (count.total==0) {
//		if (!quiet) printf("No overflow events generated.\n");
//		test_fail(test_string);
//	}

	if (count.hup!=0) {
		if (!quiet) printf("Unexpected POLL_HUP overflow.\n");
		test_fail(test_string);
	}

	if (count.in==0) {
		if (!quiet) printf("Error: POLL_IN of 0.\n");
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}

