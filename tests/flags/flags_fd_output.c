/* flags_fd_output.c  */
/* Test PERF_FLAG_FD_OUTPUT functionality */

/* Note: this functionality seems to have been broken since 2.6.35
	ac9721f3f54b27a16c7e1afb2481e7ee95a70318			*/

/* It was proposed for removal in this thread:
	https://lkml.org/lkml/2015/1/8/285
   but nothing ever happened.  */

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
#include "test_utils.h"
#include "perf_helpers.h"
#include "perf_barrier.h"
#include "instructions_testcode.h"

static int quiet;

int read_mmap_size(void *our_mmap) {

	struct perf_event_mmap_page *control_page = our_mmap;
	long long head;
	int size;
	static long long prev_head=0;

//	unsigned char *data;

//	void *data_mmap=our_mmap+getpagesize();

	if (control_page==NULL) {
		fprintf(stderr,"ERROR mmap page NULL\n");
		return -1;
        }

	head=control_page->data_head;
	rmb(); /* Must always follow read of data_head */

	size=head-prev_head;

	if (!quiet) {
		printf("Head: %lld Prev_head=%lld\n",head,prev_head);
		printf("%d new bytes\n",size);
	}

	return size;


}


int main(int argc, char** argv) {

	int fd1,fd2;
	struct perf_event_attr pe1,pe2;

	int result;
	long long single=0,both=0;

	char test_string[]="Testing PERF_FLAG_FD_OUTPUT flag...";
	char *our_mmap;

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing PERF_FLAG_FD_OUTPUT flag.\n");
		printf("Note: this feature has been broken since 2.6.35\n");
	}

	/**********************************/
	/* test 1, creating group, normal */
	/**********************************/

	if (!quiet) {
		printf("1. Testing normal group leader\n");
	}

	/* Create group leader */

	memset(&pe1,0,sizeof(struct perf_event_attr));
	pe1.type=PERF_TYPE_HARDWARE;
	pe1.size=sizeof(struct perf_event_attr);
	pe1.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe1.disabled=1;
	pe1.exclude_kernel=1;
	pe1.exclude_hv=1;
	pe1.sample_period=50000;
	arch_adjust_domain(&pe1,quiet);

	/* Create group leader */
	fd1=perf_event_open(&pe1,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Unexpected error %s\n",strerror(errno));

		}
		test_fail(test_string);
	}

	our_mmap=mmap(NULL, 8192,
		PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);
	if (our_mmap==MAP_FAILED) {
		fprintf(stderr,"mmap() failed %s!\n",strerror(errno));
		test_fail(test_string);
	}


	/* Create group member */

	memset(&pe2,0,sizeof(struct perf_event_attr));
	pe2.type=PERF_TYPE_HARDWARE;
	pe2.size=sizeof(struct perf_event_attr);
	pe2.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe2.disabled=0;
	pe2.exclude_kernel=1;
	pe2.exclude_hv=1;
	arch_adjust_domain(&pe2,quiet);

	fd2=perf_event_open(&pe2,0,-1,fd1,0);
	if (fd2<0) {
		if (!quiet) {
			fprintf(stderr,"Unexpected error %s\n",strerror(errno));

		}
		test_fail(test_string);
	}

	/* start */
	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd2, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);

	/* million */
	result=instructions_million();

	/* stop */
	ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);

	if (result==CODE_UNIMPLEMENTED) {
		fprintf(stderr,"\tCode unimplemented\n");
		test_skip(test_string);
	}

	/* read mmap */

	single=read_mmap_size(our_mmap);

	/* close */
	munmap(our_mmap,8192);
	close(fd1);
	close(fd2);

	/************************************************/
	/* test 2, reading group, PERF_FLAG_FD_NO_GROUP */
	/************************************************/

	if (!quiet) {
		printf("2. Testing with PERF_FLAG_FD_NO_GROUP\n");
	}

	/* Create group leader */

	memset(&pe1,0,sizeof(struct perf_event_attr));
	pe1.type=PERF_TYPE_HARDWARE;
	pe1.size=sizeof(struct perf_event_attr);
	pe1.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe1.disabled=1;
	pe1.exclude_kernel=1;
	pe1.exclude_hv=1;
	pe1.sample_period=50000;
	arch_adjust_domain(&pe1,quiet);

	fd1=perf_event_open(&pe1,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Unexpected error %s\n",strerror(errno));

		}
		test_fail(test_string);
	}

	/* Create group member */

	memset(&pe2,0,sizeof(struct perf_event_attr));
	pe2.type=PERF_TYPE_HARDWARE;
	pe2.size=sizeof(struct perf_event_attr);
	pe2.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe2.disabled=0;
	pe2.exclude_kernel=1;
	pe2.exclude_hv=1;
	pe2.sample_period=50000;
	arch_adjust_domain(&pe2,quiet);

	/* This has failed since Linux 2.6.35 */
	fd2=perf_event_open(&pe2,0,-1,fd1,PERF_FLAG_FD_NO_GROUP |
					PERF_FLAG_FD_OUTPUT);
	if (fd2<0) {
		if (!quiet) {
			fprintf(stderr,"Unexpected error creating member event %s\n",strerror(errno));

		}
		test_known_kernel_bug(test_string);
	}


	our_mmap=mmap(NULL, 8192,
		PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);
	if (our_mmap==MAP_FAILED) {
		fprintf(stderr,"mmap() failed %s!\n",strerror(errno));
		test_fail(test_string);
	}

	/* start */
	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd2, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);

	/* million */
	result=instructions_million();

	/* stop */
	ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);

	if (result==CODE_UNIMPLEMENTED) {
		fprintf(stderr,"\tCode unimplemented\n");
		test_skip(test_string);
	}

	/* read */

	both=read_mmap_size(our_mmap);

	/* close */

	munmap(our_mmap,8192);
	close(fd1);
	close(fd2);

	if (both<=single) {

		if (!quiet) {
			fprintf(stderr,"Expected to get more samples when both together\n");
		}

		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}

