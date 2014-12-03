/* exclude_host.c  */
/* Test the exclude_host bit */

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

#define EVENTS 1

int main(int argc, char** argv) {

	int ret,fd1,quiet;
	int result;
	long long buffer[EVENTS];
	int errors=0;
	long long with,without;

	struct perf_event_attr pe;

	char test_string[]="Testing exclude_host...";

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing the exclude_host bit.\n");
	}

	/*********************************/
	/* Test 1: 1 event, exclude_host = 0 */
	/*********************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.exclude_host=0;

//	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	instructions_million();

	/* disable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	result=read(fd1,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("With attr.exclude_host=0\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	without=buffer[0];

	close(fd1);

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!  Empty result\n");
		errors++;
	}

	/*********************************/
	/* Test 2: 1 event, exclude_host = 1 */
	/*********************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.exclude_host=1;

//	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	instructions_million();

	/* disable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	result=read(fd1,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("With attr.exclude_host=1\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	close(fd1);

	if (buffer[0]==0) {
		if (!quiet) printf("\tError! Empty result\n");
		errors++;
	}

	with=buffer[0];

	/* There's not a really good way to test for this */

	if ((without)&&(with==0)) {

		if (!quiet) {
			printf("Probably running natively, attr.exclude_host=1 gives 0 result\n");
		}
//		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}

