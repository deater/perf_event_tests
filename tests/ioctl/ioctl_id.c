/* ioctl_id.c                               */
/* Test the PERF_IOC_ID ioctl               */

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

char test_string[]="Testing ioctl(PERF_EVENT_IOC_ID)...";

int main(int argc, char** argv) {

	int fd[2];
	int quiet;
	int read_result,result;
	long long insn_data[4],id;

	struct perf_event_attr pe;

	quiet=test_quiet();

	if (!quiet) {
		printf("This tests the PERF_EVENT_IOC_ID ioctl.\n\n");
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

	ioctl(fd[0], PERF_EVENT_IOC_RESET, 0);
	ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	instructions_million();

	ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	read_result=read(fd[0],insn_data,2*sizeof(long long));

	if (read_result!=2*sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	result=ioctl(fd[0], PERF_EVENT_IOC_ID, &id);
	if (result!=0) {
		if (errno==ENOTTY) {
			test_skip(test_string);
		}
		fprintf(stderr,"\tError with ioctl %s\n",
			strerror(errno));
	}

	close(fd[0]);



	if (!quiet) {
		printf("\t%lld instructions id=%lld id from ioctl=%lld\n",
			insn_data[0],insn_data[1],id);
	}

	if (insn_data[1]!=id) {
		fprintf(stderr,"ioctl result doesn't match read result\n");
		test_fail(test_string);
	}


	/* TODO -- test group leader vs not */

	test_pass(test_string);

	return 0;
}
