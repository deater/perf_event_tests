/* This file attempts to test the hardware breakpoint support */

/* This was not added until 2.6.33 */

/* by Vince Weaver, vincent.weaver _at_ maine.edu        */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#include "perf_event.h"
#include "hw_breakpoint.h"

#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

static int quiet=0;
static char test_string[]="Testing hardware breakpoints...";
static int fd;

int test_function(int a, int b, int quiet) __attribute__((noinline));

int test_function(int a, int b, int quiet) {

	int c;

	/* The time thing is there to keep the compiler */
	/* from optimizing this away.                   */

	c=a+b+rand();
	if (!quiet) printf("\t\tFunction: %d\n",c);
	return c;

}

volatile int test_variable=5;

#define EXECUTIONS 10
#define READS 15
#define WRITES 20

static void alarm_handler(int signum,siginfo_t *oh, void *blah) {

	if (!quiet) printf("Error!  Took too long!\n");
	test_fail(test_string);
}


int main(int argc, char **argv) {

	struct perf_event_attr pe;
	int i, sum=0, read_result, passes=0;
	long long count;

	void *address;

	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = alarm_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGALRM, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}


	/* Setup quiet variable */
	quiet=test_quiet();
	if (!quiet) {
		printf("This test checks that hardware breakpoints work.\n");
	}

	/* Set address of breakpoint */
	address=test_function;


	/*******************************/
	/* Test execution breakpoint   */
	/*******************************/

	if (!quiet) {
		printf("\tTesting HW_BREAKPOINT_X\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_BREAKPOINT;
	pe.size=sizeof(struct perf_event_attr);

	/* setup for an execution breakpoint */
	pe.config=0;
	pe.bp_type=HW_BREAKPOINT_X;
	pe.bp_addr=(unsigned long)address;
	pe.bp_len=sizeof(long);

	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		if (!quiet) {
			printf("\t\tError opening leader %llx %s\n",
				pe.config,strerror(errno));
		}
		goto skip_execs;
	}


	/* Setup timeout, as on ARM/pi3 4.9 it hangs forever? */
	alarm(5);

	ioctl(fd, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

	/* Access a function 10 times */
	for(i=0;i<EXECUTIONS;i++) {
		sum+=test_function(i,sum,quiet);
	}

	if (!quiet) {
		printf("\t\tSum=%d\n",sum);
	}

	ioctl(fd, PERF_EVENT_IOC_DISABLE,0);

	/* Disable alarm */
	alarm(0);


	read_result=read(fd,&count,sizeof(long long));

	if (read_result!=sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",
				read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\t\tTried %d calls, found %lld times, sum=%d\n",
			EXECUTIONS,count,sum);

		if (count!=EXECUTIONS) {
			fprintf(stderr,"\tWrong number of executions "
					"%lld != %d\n",count,EXECUTIONS);
			test_fail(test_string);
		}
	}

	close(fd);
	passes++;

skip_execs:

	/* Urgh, it's hard to force gcc 4.8+ to not optimize */
	/* this all away.                                    */
	if (!quiet) printf("Testing hardware breakpoints (%d)...",sum);

	/*******************************/
	/* Test write breakpoint       */
	/*******************************/

	if (!quiet) printf("\tTesting HW_BREAKPOINT_W\n");

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_BREAKPOINT;
	pe.size=sizeof(struct perf_event_attr);

	/* setup for an execution breakpoint */
	pe.config=0;
	pe.bp_type=HW_BREAKPOINT_W;
	pe.bp_addr=(long)&test_variable;
	pe.bp_len=HW_BREAKPOINT_LEN_4;

	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		if (!quiet) {
			printf("\t\tError opening leader %llx %s\n",
				pe.config,strerror(errno));
		}
		goto skip_writes;
	}

	ioctl(fd, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

	/* Write a variable WRITES times */
	for(i=0;i<WRITES;i++) {
		test_variable=sum+time(NULL);
		if (!quiet) printf("\t\tWrote %d\n",test_variable);
	}

	ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
	read_result=read(fd,&count,sizeof(long long));

	if (read_result!=sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\t\tTried %d writes, found %lld times, sum=%d\n",
			WRITES,count,sum);
	}

	if (count!=WRITES) {
		fprintf(stderr,"\tWrong number of writes "
				"%lld != %d\n",count,WRITES);
		test_fail(test_string);
	}

	close(fd);
	passes++;
skip_writes:

	/* Urgh, it's hard to force gcc 4.8+ to not optimize */
	/* this all away.                                    */
	if (!quiet) printf("Testing hardware breakpoints (%d)...",
			test_variable);


	/*******************************/
	/* Test read breakpoint        */
	/*******************************/

	if (!quiet) printf("\tTesting HW_BREAKPOINT_R\n");

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_BREAKPOINT;
	pe.size=sizeof(struct perf_event_attr);

	/* setup for an execution breakpoint */
	pe.config=0;
	pe.bp_type=HW_BREAKPOINT_R;
	pe.bp_addr=(long)&test_variable;
	pe.bp_len=HW_BREAKPOINT_LEN_4;

	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		if (!quiet) {
			printf("\t\tError opening leader %llx %s\n",
				pe.config,strerror(errno));
			printf("\t\tRead breakpoints probably not supported, "
				"skipping\n");
		}
		goto skip_reads;

	}

	ioctl(fd, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

	/* Read a variable READS times */
	for(i=0;i<READS;i++) {
		sum+=test_variable;
		if (!quiet) printf("\t\tRead %d\n",sum);
	}

	ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
	read_result=read(fd,&count,sizeof(long long));

	if (read_result!=sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\t\tTrying %d reads, found %lld times, sum=%d\n",
			READS,count,sum);
	}

	if (count!=READS) {
		fprintf(stderr,"\tWrong number of reads "
			"%lld != %d\n",count,READS);
		test_fail(test_string);
	}

	close(fd);
	passes++;

skip_reads:

	if (passes>0) {
		test_pass(test_string);
	} else {
		test_skip(test_string);
	}

	return 0;
}
