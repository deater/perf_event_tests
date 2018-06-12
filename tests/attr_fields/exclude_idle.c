/* exclude_idle.c  */
/* Test the exclude_idle bit */

/* FIXME:
	a proper test would do something like the following.
	Set up a sampling event.
	Do system wide measuring.
	See if any of the samples appear in the idle task.
	That's a bit complex.

*/

/* by Vince Weaver   vincent.weaver@maine.edu */

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

	struct perf_event_attr pe;

	char test_string[]="Testing exclude_idle...";

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing the exclude_idle bit.\n");
	}

	/*********************************/
	/* Test 1: 1 event, exclude_idle = 0 */
	/*********************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.exclude_idle=0;

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
		printf("With attr.exclude_idle=0\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

//	without=buffer[0];

	close(fd1);

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!  Empty result\n");
		errors++;
	}

	/*********************************/
	/* Test 2: 1 event, exclude_idle = 1 */
	/*********************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.exclude_idle=1;

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
		printf("With attr.exclude_idle=1\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	close(fd1);

	if (buffer[0]==0) {
		if (!quiet) printf("\tError! Empty result\n");
		errors++;
	}


	/*********************************/
	/* Test 3: 1 SW event, exclude_idle = 0 */
	/*********************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_SOFTWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_SW_CPU_CLOCK;
	pe.exclude_idle=0;

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
		printf("With attr.exclude_idle=0\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

//	without=buffer[0];

	close(fd1);

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!  Empty result\n");
		errors++;
	}

	/*********************************/
	/* Test 4: 1 sw event, exclude_idle = 1 */
	/*********************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_SOFTWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_SW_CPU_CLOCK;
	pe.exclude_idle=1;

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
		printf("With attr.exclude_idle=1\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	close(fd1);

	if (buffer[0]==0) {
		if (!quiet) printf("\tError! Empty result\n");
		errors++;
	}


	test_pass(test_string);

	return 0;
}
