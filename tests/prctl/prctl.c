/* prctl.c					*/
/* Test the perf-related prctl() commands	*/

/* by Vince Weaver   vincent.weaver _at_ maine.edu  */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include <errno.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"
#include "instructions_testcode.h"

static char test_string[]="Testing prctl()...";

int main(int argc, char** argv) {

	int fd[2];
	int quiet;
	int read_result;
	long long insn_data[4],old[4];

	struct perf_event_attr pe;

	quiet=test_quiet();

	if (!quiet) {
		printf("This tests prctl().\n\n");
	}

	/********************************************************/
	/* Test 1: One process, One Event			*/
	/********************************************************/

	if (!quiet) {
		printf("One process, One event\n");
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

	prctl(PR_TASK_PERF_EVENTS_ENABLE);

	instructions_million();

	prctl(PR_TASK_PERF_EVENTS_DISABLE);

	read_result=read(fd[0],insn_data,2*sizeof(long long));

	if (read_result!=2*sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\t+ Read (should be 1 million): %lld\n",insn_data[0]);
	}
	old[0]=insn_data[0];

	instructions_million();

	read_result=read(fd[0],insn_data,2*sizeof(long long));

	if (read_result!=2*sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tReading again after disable\n");
		printf("\t+ Read Instructions (should match last): %lld (%lld)\n",
			insn_data[0],old[0]);
	}

	if (old[0]==0) {
		test_fail(test_string);
	}

	if (old[0]!=insn_data[0]) {
		test_fail(test_string);
	}

	close(fd[0]);





	/********************************************************/
	/* Test 2: One process, Two Events			*/
	/********************************************************/

	if (!quiet) {
		printf("One process, Two events\n");
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

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_BUS_CYCLES;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.read_format=PERF_FORMAT_ID;

	arch_adjust_domain(&pe, quiet);

	fd[1]=perf_event_open(&pe,0,-1,-1,0);
	if (fd[1]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	prctl(PR_TASK_PERF_EVENTS_ENABLE);

	instructions_million();

	prctl(PR_TASK_PERF_EVENTS_DISABLE);

	read_result=read(fd[0],insn_data,2*sizeof(long long));

	if (read_result!=2*sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\t+ Read Instructions (should be 1 million): %lld\n",insn_data[0]);
		printf("\t+ Read Cycles: %lld\n",insn_data[1]);
	}
	old[0]=insn_data[0];
	old[1]=insn_data[1];

	instructions_million();

	read_result=read(fd[0],insn_data,2*sizeof(long long));

	if (read_result!=2*sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tReading again after disable\n");
		printf("\t+ Read Instructions (should match last): %lld (%lld)\n",insn_data[0],old[0]);
		printf("\t+ Read Cycles (should match last): %lld (%lld)\n",insn_data[1],old[1]);
	}

	if (old[0]==0) {
		test_fail(test_string);
	}
	if (old[1]==0) {
		test_fail(test_string);
	}

	if (old[0]!=insn_data[0]) {
		test_fail(test_string);
	}
	if (old[1]!=insn_data[1]) {
		test_fail(test_string);
	}

	close(fd[0]);


	/********************************************************/
	/* Test 3: One process, Two Events in group		*/
	/********************************************************/

	if (!quiet) {
		printf("One process, Two events in group\n");
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

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_BUS_CYCLES;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.read_format=PERF_FORMAT_ID;

	arch_adjust_domain(&pe, quiet);

	fd[1]=perf_event_open(&pe,0,-1,fd[0],0);
	if (fd[1]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	prctl(PR_TASK_PERF_EVENTS_ENABLE);

	instructions_million();

	prctl(PR_TASK_PERF_EVENTS_DISABLE);

	read_result=read(fd[0],insn_data,2*sizeof(long long));

	if (read_result!=2*sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\t+ Read Instructions (should be 1 million): %lld\n",insn_data[0]);
		printf("\t+ Read Cycles: %lld\n",insn_data[1]);
	}
	old[0]=insn_data[0];
	old[1]=insn_data[1];

	instructions_million();

	read_result=read(fd[0],insn_data,2*sizeof(long long));

	if (read_result!=2*sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tReading again after disable\n");
		printf("\t+ Read Instructions (should match last): %lld (%lld)\n",insn_data[0],old[0]);
		printf("\t+ Read Cycles (should match last): %lld (%lld)\n",insn_data[1],old[1]);
	}

	if (old[0]==0) {
		test_fail(test_string);
	}
	if (old[1]==0) {
		test_fail(test_string);
	}

	if (old[0]!=insn_data[0]) {
		test_fail(test_string);
	}
	if (old[1]!=insn_data[1]) {
		test_fail(test_string);
	}

	close(fd[0]);




	test_pass(test_string);


	return 0;
}
