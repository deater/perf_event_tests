/* This file attempts to test if non-existent events fail */

/* The reason for this test is that in the various linux-kernel  */
/*   pre-defined event definitions, sometimes unavailable events */
/*   are implicitly set to 0 and sometimes set to -1.            */
/*   This test makes sure both fail properly.                    */

/* by Vince Weaver, vincent.weaver _at_ maine.edu                */


static char test_string[]="Testing if non-existent events fail...";
static int quiet=0;
static int fd;

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

int main(int argc, char **argv) {

	struct perf_event_attr pe;
	int correct_fails=0,expected_fails=0,cpu;
	int zero_event=0,negone_event=0;
	int zero_type=0,negone_type=0;
	char zero_name[BUFSIZ],negone_name[BUFSIZ];

	quiet=test_quiet();

	cpu=detect_processor();

	/* TODO:
		PROCESSOR_PENTIUM_PRO  1
		PROCESSOR_PENTIUM_II   2
		PROCESSOR_PENTIUM_III  3
		PROCESSOR_PENTIUM_4    4
		PROCESSOR_PENTIUM_M    5
		PROCESSOR_COREDUO      6
		PROCESSOR_AMD_FAM15H  19
		PROCESSOR_POWER3      103
		PROCESSOR_POWER4      104
		PROCESSOR_POWER5      105
		PROCESSOR_POWER6      106
		PROCESSOR_POWER7      107
	*/

	if (( cpu==PROCESSOR_CORTEX_A8) ||
		( cpu==PROCESSOR_CORTEX_A9)) {

		/* ARM does things a bit differently;		*/
		/* it has HW_OP_UNSUPPORTED,			*/
		/* and CACHE_OP_UNSUPPORTED, both set to 0xffff */

		/* HW_OP_UNSUPPORTED */
		zero_type=PERF_TYPE_HARDWARE;
		zero_event=PERF_COUNT_HW_BUS_CYCLES;
		strncpy(zero_name,"bus-cycles",BUFSIZ);

		/* CACHE_OP_UNSUPPORTED */
		negone_type=PERF_TYPE_HW_CACHE;
		negone_event= PERF_COUNT_HW_CACHE_BPU |
			( PERF_COUNT_HW_CACHE_OP_PREFETCH <<8) |
			(PERF_COUNT_HW_CACHE_RESULT_ACCESS <<16);
		strncpy(negone_name,"branch-prefetches",BUFSIZ);

	}
	else if (	( cpu==PROCESSOR_ATOM) ||
			( cpu==PROCESSOR_CORE2) ||
			( cpu==PROCESSOR_NEHALEM) ||
			( cpu==PROCESSOR_NEHALEM_EX) ||
			( cpu==PROCESSOR_WESTMERE) ||
			( cpu==PROCESSOR_WESTMERE_EX) ||
			( cpu==PROCESSOR_SANDYBRIDGE) ||
			( cpu==PROCESSOR_IVYBRIDGE) ||
			( cpu==PROCESSOR_HASWELL) ||
			( cpu==PROCESSOR_HASWELL_EP) ||
			( cpu==PROCESSOR_BROADWELL) ||
			( cpu==PROCESSOR_SKYLAKE) ||
			( cpu==PROCESSOR_KABYLAKE)) {


		zero_type=PERF_TYPE_HW_CACHE;
		zero_event= PERF_COUNT_HW_CACHE_DTLB |
			( PERF_COUNT_HW_CACHE_OP_PREFETCH <<8) |
			(PERF_COUNT_HW_CACHE_RESULT_ACCESS <<16);
		strncpy(zero_name,"dTLB-prefetches",BUFSIZ);

		negone_type=PERF_TYPE_HW_CACHE;
		negone_event= PERF_COUNT_HW_CACHE_BPU |
			( PERF_COUNT_HW_CACHE_OP_PREFETCH <<8) |
			(PERF_COUNT_HW_CACHE_RESULT_ACCESS <<16);
		strncpy(negone_name,"branch-prefetches",BUFSIZ);
	}
	else if (	(cpu==PROCESSOR_K7) ||
			(cpu==PROCESSOR_K8) ||
			(cpu==PROCESSOR_AMD_FAM10H) ||
			(cpu==PROCESSOR_AMD_FAM11H) ||
			(cpu==PROCESSOR_AMD_FAM12H) ||
			(cpu==PROCESSOR_AMD_FAM13H) ||
			(cpu==PROCESSOR_AMD_FAM14H) ||
			(cpu==PROCESSOR_AMD_FAM15H) ||
			(cpu==PROCESSOR_AMD_FAM16H) ||
			(cpu==PROCESSOR_AMD_FAM17H)) {

		/* event implicitly set to 0 */
		zero_type=PERF_TYPE_HARDWARE;
		zero_event=PERF_COUNT_HW_BUS_CYCLES;
		strncpy(zero_name,"bus-cycles",BUFSIZ);

		/* event set to -1 in arch/x86/kernel/cpu/perf_event_amd.h */
		negone_type=PERF_TYPE_HW_CACHE;
		negone_event= PERF_COUNT_HW_CACHE_BPU |
			( PERF_COUNT_HW_CACHE_OP_PREFETCH <<8) |
			(PERF_COUNT_HW_CACHE_RESULT_ACCESS <<16);
		strncpy(negone_name,"branch-prefetches",BUFSIZ);
	}
	else {
		test_skip(test_string);
	}

	if (!quiet) {
		printf("This test checks if non-existent events fail\n\n");
		printf("First testing non-existent (set to 0) event: ");
		printf("%s\n",zero_name);
		printf("Should return ENOENT\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=zero_type;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=zero_event;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		if (errno==ENOENT) {
			if (!quiet) {
				printf("\tCorrectly failed with fd=%d error %d == %d %s!\n",
					fd,ENOENT,errno,strerror(errno));
			}
			correct_fails++;
		} else {
			printf("\tFailed with wrong error fd=%d error %d != %d %s!\n",
				fd,errno,ENOENT,strerror(errno));
		}
	}
	else {
		if (!quiet) printf("\tERROR: we succeded!\n");
	}

	expected_fails++;

	close(fd);

	if (!quiet) {
		printf("\nNow testing non-existent (set to -1) event: ");
		printf("%s\n",negone_name);
		printf("Should return EINVAL\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=negone_type;
	pe.size=sizeof(struct perf_event_attr);

	pe.config=negone_event;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		if (errno==EINVAL) {
			if (!quiet) {
				printf("\tCorrectly failed with fd=%d error %d == %d %s!\n",
					fd,errno,EINVAL,strerror(errno));
			}
			correct_fails++;
		}
		else {
			printf("\tFailed with wrong error fd=%d error %d != %d %s!\n",
				fd,errno,EINVAL,strerror(errno));
		}
	}
	else {
		if (!quiet) printf("\tERROR: we succeded!\n");
	}

	expected_fails++;

	close(fd);

	if (!quiet) printf("\n");

	if (correct_fails!=expected_fails) {
		if (!quiet) printf("Not enough failures!\n");
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}
