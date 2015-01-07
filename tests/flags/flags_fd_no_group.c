/* flags_fd_no_group.c  */
/* Test PERF_FLAG_FD_NO_GROUP functionality */

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
#include "instructions_testcode.h"


int main(int argc, char** argv) {

	int fd1,fd2,quiet;
	struct perf_event_attr pe1,pe2;

	int result,read_result;
	long long count[2];

	char test_string[]="Testing PERF_FLAG_FD_NO_GROUP flag...";

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing PERF_FLAG_FD_NO_GROUP flag.\n");
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
	arch_adjust_domain(&pe1,quiet);

	/* Create group leader */
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

	/* read */

	read_result=read(fd1,&count[0],sizeof(long long));

	if (read_result!=sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	read_result=read(fd2,&count[1],sizeof(long long));

	if (read_result!=sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tRead %lld %lld instructions\n",count[0],count[1]);
	}

	if (((count[0]-count[1])>5) || ((count[0]-count[1])<-5)) {
		if (!quiet) {
			printf("Error!  Counts should be same because in a group\n");
		}
		test_fail(test_string);
	}

	/* close */
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
	arch_adjust_domain(&pe2,quiet);

	fd2=perf_event_open(&pe2,0,-1,fd1,PERF_FLAG_FD_NO_GROUP);
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

	/* read */

	read_result=read(fd1,&count[0],sizeof(long long));

	if (read_result!=sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	read_result=read(fd2,&count[1],sizeof(long long));

	if (read_result!=sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tRead %lld %lld instructions\n",count[0],count[1]);
	}

	if (((count[0]-count[1])<5) && ((count[0]-count[1])>-5)) {
		if (!quiet) {
			printf("Error!  Counts should be further apart due to flag settings\n");
		}
		test_fail(test_string);
	}

	/* close */

	close(fd1);
	close(fd2);

	test_pass(test_string);

	return 0;
}

