/* ioctl_11_modify_attributes.c				*/
/* Test the PERF_IOC_MODIFY_ATTRIBUTES ioctl		*/

/* Current only works for breakpoint events */
/* Used to modify breakpoint address w/o opening/closing */

/* Introduced Linux 4.17, 32ff77e8cc9e66cc4fb38098f64fd54cc8f54573 */

/* by Vince Weaver   vincent.weaver@maine.edu		*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>

#include <errno.h>

#include "perf_event.h"
#include "hw_breakpoint.h"

#include "perf_helpers.h"
#include "test_utils.h"
#include "instructions_testcode.h"

static char test_string[]="Testing ioctl(PERF_EVENT_IOC_MODIFY_ATTRIBUTES)...";

int main(int argc, char** argv) {

	int fd[2];
	int quiet;
	int read_result;
	long long insn_data[4];
	int result;

	struct perf_event_attr pe,pe2;

	quiet=test_quiet();

	if (!quiet) {
		printf("This tests the PERF_EVENT_IOC_MODIFY_ATTRIBUTES ioctl.\n\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe, quiet);

	fd[0]=perf_event_open(&pe,0,-1,-1,0);
	if (fd[0]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	instructions_million();

	ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	read_result=read(fd[0],insn_data,sizeof(long long));

	if (read_result!=sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	/* Attempt to modify non-breakpoint event */
	/* This should fail */

	memset(&pe2,0,sizeof(struct perf_event_attr));

	pe2.type=PERF_TYPE_HARDWARE;
	pe2.config=PERF_COUNT_HW_CPU_CYCLES;
	pe2.disabled=1;
	pe2.exclude_kernel=1;
	pe2.exclude_hv=1;

	arch_adjust_domain(&pe2, quiet);

	result=ioctl(fd[0],PERF_EVENT_IOC_MODIFY_ATTRIBUTES,&pe2);
	if (result<0) {
		if (errno==EOPNOTSUPP) {
			if (!quiet) {
				printf("+ Failed as expected on non-breakpoint event\n");
			}
		}
		else {
			fprintf(stderr,"Unknown failure %s\n",
				strerror(errno));
			test_fail(test_string);
		}
	}


	close(fd[0]);

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
	pe.bp_addr=(unsigned long)(main);
	pe.bp_len=sizeof(long);

	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd[0]=perf_event_open(&pe,0,-1,-1,0);
	if (fd[0]<0) {
		if (!quiet) {
			printf("\t\tError opening leader %llx %s\n",
				pe.config,strerror(errno));
		}

	}

	ioctl(fd[0], PERF_EVENT_IOC_RESET, 0);
	ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);


	ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);


	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_BREAKPOINT;
	pe.size=sizeof(struct perf_event_attr);

	/* setup for an execution breakpoint */
	pe.config=0;
	pe.bp_type=HW_BREAKPOINT_X;
	pe.bp_addr=(unsigned long)(main);
	pe.bp_len=sizeof(long);

	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	result=ioctl(fd[0],PERF_EVENT_IOC_MODIFY_ATTRIBUTES,&pe);
	if (result<0) {
		fprintf(stderr,"ioctl didn't work %s\n",
			strerror(errno));
		test_fail(test_string);
	}

	close(fd[0]);

	test_pass(test_string);

	return 0;
}
