/* This tests using the intel Fixed Counter 0 */
/* Which maps to retired_instructions         */
/* Support added in Linux 2.6.X */
/* Note: bugs where count not same as same counter in general purpose counter */

/* by Vince Weaver, vincent.weaver@maine.edu          */


char test_string[]="Testing fixed counter 0 event...";
int quiet=0;
int fd;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

int main(int argc, char **argv) {

	struct perf_event_attr pe;
	void *mmap_result;
	int result;
	long long values[256];

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks the intel fixed counter 0 %d\n",getpid());
	}

	sleep(1);

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=80;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.sample_period=0xc0000000000000bdULL;
	pe.sample_type=PERF_SAMPLE_TID|PERF_SAMPLE_ID|PERF_SAMPLE_CPU|PERF_SAMPLE_PERIOD|PERF_SAMPLE_STREAM_ID|PERF_SAMPLE_RAW|PERF_SAMPLE_STACK_USER|PERF_SAMPLE_DATA_SRC|PERF_SAMPLE_IDENTIFIER|PERF_SAMPLE_TRANSACTION; /* 3a7c2 */
        pe.read_format=PERF_FORMAT_TOTAL_TIME_RUNNING|PERF_FORMAT_GROUP; /* a */

//	pe.exclude_user=1;
        pe.exclude_kernel=1;
        pe.exclude_hv=1;
        pe.mmap=1;
        pe.comm=1;
        pe.enable_on_exec=1;
        pe.task=1;
        pe.watermark=1;
  //      pe.precise_ip=0; /* arbitrary skid */
       pe.exclude_callchain_kernel=1;
        pe.exclude_callchain_user=1;
        pe.wakeup_watermark=254;
        pe.branch_sample_type=PERF_SAMPLE_BRANCH_USER|PERF_SAMPLE_BRANCH_ANY_CALL|PERF_SAMPLE_BRANCH_IND_CALL|PERF_SAMPLE_BRANCH_ABORT_TX;
        pe.sample_regs_user=57;
        pe.sample_stack_user=510;

	pe.disabled=1;
	pe.pinned=1;

//	fd=perf_event_open(&pe,0,0,-1,PERF_FLAG_FD_OUTPUT);

	fd=perf_event_open(&pe,0,-1,-1,PERF_FLAG_FD_OUTPUT);
	if (fd<0) {
		fprintf(stderr,"Error opening leader %llx %s\n",pe.config,strerror(errno));
		test_fail(test_string);
	}

	mmap_result=mmap(NULL, 0x2ae6,PROT_READ|PROT_WRITE, MAP_SHARED,fd, 0);

	ioctl(fd,PERF_EVENT_IOC_ENABLE,0);

	int i;
	result=0;
	for(i=0;i<1000;i++) {
		result+=instructions_million();
	}

	ioctl(fd,PERF_EVENT_IOC_DISABLE,0);

	ioctl(fd,PERF_EVENT_IOC_ENABLE,0);
	result+=instructions_million();
	ioctl(fd,PERF_EVENT_IOC_DISABLE,0);

	read(fd,values,128);

//	munmap(mmap_result,10982);
	close(fd);

	printf("result: %d - %lld %lld %lld\n",result,values[0],values[1],values[2]);

	printf("Years: %lld\n",(((((pe.sample_period / 4000000000ULL)/2ULL)/3600ULL)/24ULL)/365ULL));

	{
		unsigned long long usp;
		signed long long period;
		signed long long period_left=0,left=0;

		usp=pe.sample_period;
		period=(signed long long)usp;
		printf("%lld %lld %llx\n",usp,period,period);

		if (left<=-period) {
			printf("Less than\n");
			left = period;
			period_left=left;
//			hwc->last_period = period;
		}

		if (left<=0) {
			printf("Negative\n");
			left+=period;
			period_left=left;
			printf("%lld %llx\n",period_left,period_left);
			left+=period;
			period_left=left;
			printf("%lld %llx\n",period_left,period_left);
		}

	}

	test_pass(test_string);

	return 0;
}
