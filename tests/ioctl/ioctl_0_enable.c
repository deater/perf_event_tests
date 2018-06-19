/* ioctl_0_enable.c				*/
/* Test the PERF_IOC_ENABLE ioctl		*/

/* by Vince Weaver   vincent.weaver _at_ maine.edu  */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>

#include <errno.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"
#include "instructions_testcode.h"

static char test_string[]="Testing ioctl(PERF_EVENT_IOC_ENABLE)...";

int main(int argc, char** argv) {

	int fd[2];
	int quiet;
	int read_result;
	long long insn_data[4];

	struct perf_event_attr pe;

	quiet=test_quiet();

	if (!quiet) {
		printf("This tests the PERF_EVENT_IOC_ENABLE ioctl.\n\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.read_format=PERF_FORMAT_ID;

	arch_adjust_domain(&pe, quiet);

	fd[0]=perf_event_open(&pe,0,-1,-1,0);
	if (fd[0]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	instructions_million();

	ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	read_result=read(fd[0],insn_data,2*sizeof(long long));

	if (read_result!=2*sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	close(fd[0]);

	if (!quiet) {
		printf("\t%lld instructions\n",insn_data[0]);
	}

	/* TODO -- test group leader vs not */

	test_pass(test_string);

	return 0;
}
