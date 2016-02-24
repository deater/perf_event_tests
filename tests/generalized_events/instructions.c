/* This file attempts to test the retired instruction	*/
/* performance counter on various architectures, as	*/
/* implemented by the perf_event generalized event	*/
/*      instructions					*/

/* by Vince Weaver, vincent.weaver _at_ maine.edu	*/


char test_string[]="Testing \"instructions\" generalized event...";
int quiet=0;
int fd;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#define NUM_RUNS 100


   /* Test a simple loop of 1 million instructions             */
   /* Most implementations should count be correct within 1%   */
   /* This loop in in assembly language, as compiler generated */
   /* code varies too much.                                    */

void test_million(void) {

	int i,result,read_result;

	long long count,high=0,low=0,total=0,average=0;
	double error;

	if (!quiet) {
		printf("\nTesting a loop of 1 million instructions (%d times):\n",
		NUM_RUNS);
	}

	for(i=0;i<NUM_RUNS;i++) {

		ioctl(fd, PERF_EVENT_IOC_RESET, 0);
		ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

		result=instructions_million();

		ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
		read_result=read(fd,&count,sizeof(long long));

		if (result==CODE_UNIMPLEMENTED) {
			fprintf(stderr,"\tCode unimplemented\n");
			test_skip(test_string);
		}

		if (read_result!=sizeof(long long)) {
			fprintf(stderr,"\tImproper return from read: %d\n",read_result);
			test_fail(test_string);
		}

		if (count>high) high=count;
		if ((low==0) || (count<low)) low=count;
		total+=count;
	}

	average=total/NUM_RUNS;

	error=display_error(average,high,low,1000000ULL,quiet);

	if ((error > 1.0) || (error<-1.0)) {

#if defined(__PPC__)

		if(!quiet) {
			printf("If PPC is off by 50%%, this might be due to\n"
				"\"folded\" branch instructions on PPC32\n");
		}
#endif

		test_fail(test_string);
	}
}

/* Test fldcw.  Pentium 4 overcounts this instruction */

void test_fldcw(void) {

#if defined(__i386__) || (defined __x86_64__)
	int i,result,read_result;

	long long count,high=0,low=0,total=0,average=0;
	double error;

	if (!quiet) {
		printf("\nTesting a fldcw loop of 900,000 instructions (%d times):\n",
		NUM_RUNS);
	}

	for(i=0;i<NUM_RUNS;i++) {

		ioctl(fd, PERF_EVENT_IOC_RESET, 0);
		ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

		result=instructions_fldcw();

		ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
		read_result=read(fd,&count,sizeof(long long));

		if (result==CODE_UNIMPLEMENTED) {
			fprintf(stderr,"\tCode unimplemented\n");
			test_fail(test_string);
		}

		if (read_result!=sizeof(long long)) {
			fprintf(stderr,"\tUnexpected read result: %d\n",read_result);
			test_fail(test_string);
		}

		if (count>high) high=count;
		if ((low==0) || (count<low)) low=count;
		total+=count;
	}

	average=total/NUM_RUNS;

	error=display_error(average,high,low,900000ULL,quiet);

	if ((error > 1.0) || (error<-1.0)) {

		if (!quiet) {
			printf("On Pentium 4 machines, the fldcw instruction counts as 2.\n");
			printf("This will lead to an overcount of 22%%\n");
		}
		test_fail( test_string);
	}
#endif
}

/* Test rep-prefixed instructions. */
/* HW counters count this as one each, not one per repeat */

void test_rep(void) {

#if defined(__i386__) || (defined __x86_64__)
	int i,result,read_result;

	long long count,high=0,low=0,total=0,average=0;
	double error;

	if(!quiet) {
		printf("\nTesting a 16k rep loop (%d times):\n", NUM_RUNS);
	}

	for(i=0;i<NUM_RUNS;i++) {

		ioctl(fd, PERF_EVENT_IOC_RESET, 0);
		ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

		result=instructions_rep();

		ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
		read_result=read(fd,&count,sizeof(long long));

		if (result==CODE_UNIMPLEMENTED) {
			fprintf(stderr,"\tCode unimplemented\n");
			test_fail(test_string);
		}

		if (read_result!=sizeof(long long)) {
			fprintf(stderr,"\tUnexpected read result: %d\n",read_result);
			test_fail(test_string);
		}

		if (count>high) high=count;
		if ((low==0) || (count<low)) low=count;
		total+=count;
	}

	average=total/NUM_RUNS;

	error=display_error(average,high,low,6002,quiet);

	if ((error > 10.0) || (error<-10.0)) {
		if (!quiet) {
			printf("Instruction count off by more than 10%%\n");
		}
		test_fail( test_string);
	}
#endif
}


int main(int argc, char **argv) {

	struct perf_event_attr pe;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks that the \"instructions\" generalized "
			"event is working.\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		fprintf(stderr,"Error opening leader %llx %s\n",pe.config,strerror(errno));
		test_fail(test_string);
	}

	test_million();
	test_fldcw();
	test_rep();

	close(fd);

	test_pass(test_string);

	return 0;
}
