/* This file attempts to test the retired branches instruction */
/* performance counter on various architectures, as            */
/* implemented by the perf_events generalized event            */
/*    branch-misses                                                 */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu                 */

/* prior to 2.6.35 the wrong branch-misses event was used on AMD machines */


char test_string[]="Testing \"branch-misses\" generalized event...";
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
#include "branches_testcode.h"
#include "perf_helpers.h"


int main(int argc, char **argv) {

	int num_runs=100,i,read_result,result;
	int num_random_branches=500000;
	long long high=0,low=0,average=0,expected=1500000;
	struct perf_event_attr pe;

	int no_branch_event=0;

	long long count,total=0;

	quiet=test_quiet();

	/* Part 1 */

	if (!quiet) {
		printf("\nPart 1\n");

		printf("Testing a loop with %lld branches (%d times):\n",
			expected,num_runs);
		printf("\tOn a simple loop like this, miss rate "
			"should be very small.\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_BRANCH_MISSES;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		if (!quiet) fprintf(stderr,"Error opening leader %llx\n",pe.config);
		test_fail(test_string);
	}

	for(i=0;i<num_runs;i++) {
		ioctl(fd, PERF_EVENT_IOC_RESET, 0);
		ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

		result=branches_testcode();

		ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
		read_result=read(fd,&count,sizeof(long long));

		if (result==CODE_UNIMPLEMENTED) {
			if (!quiet) {
				printf("\tNo test code for this architecture\n");
			}
			test_skip(test_string);
		}

		if (read_result!=sizeof(long long)) {
			if (!quiet) {
				printf("Error extra data in read %d\n",
					read_result);
			}
			test_fail(test_string);
		}

		if (count>high) high=count;
		if ((low==0) || (count<low)) low=count;
		total+=count;
	}

	average=(total/num_runs);

	if (!quiet) printf("\tAverage number of branch misses: %lld\n",average);

	if (average>1000) {
		if (!quiet) printf("Branch miss rate too high\n");
		test_fail(test_string);
	}

	close(fd);

	/**********/
	/* Part 2 */

	if (!quiet) {
		printf("\nPart 2\n");
	}

	high=0; low=0; total=0;

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening branch_instructions event\n");
		}
		no_branch_event=1;
		//test_fail(test_string);
	}

	if (!no_branch_event) {

	for(i=0;i<num_runs;i++) {

		ioctl(fd, PERF_EVENT_IOC_RESET, 0);
		ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

		result=random_branches_testcode(num_random_branches,1);

		ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
		read_result=read(fd,&count,sizeof(long long));

		if (count>high) high=count;
		if ((low==0) || (count<low)) low=count;
		total+=count;
	}

	average=total/num_runs;

	expected=average;
	if (!quiet) {
		printf("\nTesting a function that branches based on a random number\n");
		printf("   The loop has %lld branches.\n",expected);
		printf("   %d are random branches; %d of those were taken\n",num_random_branches,result);
	}
	close(fd);

	}

	high=0; low=0; total=0;

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_BRANCH_MISSES;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	for(i=0;i<num_runs;i++) {

		ioctl(fd, PERF_EVENT_IOC_RESET, 0);
		ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

		result=random_branches_testcode(num_random_branches,1);

		ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
		read_result=read(fd,&count,sizeof(long long));

		if (count>high) high=count;
		if ((low==0) || (count<low)) low=count;
		total+=count;
	}

	average=total/num_runs;

	if (!quiet) {
		printf("\nOut of %lld branches, %lld were mispredicted\n",expected,average);
		printf("Assuming a good random number generator and no freaky luck\n");
		printf("The mispredicts should be roughly between %d and %d\n",
		num_random_branches/4,(num_random_branches/4)*3);
	}

	if ( average < (num_random_branches/4)) {
		if (!quiet) printf("Mispredicts too low\n");
		test_fail(test_string);
	}

	if (average > (num_random_branches/4)*3) {
		if (!quiet) printf("Mispredicts too high\n");
		test_fail(test_string);
	}

	if (!quiet) printf("\n");

	test_pass( test_string );

	return 0;
}
