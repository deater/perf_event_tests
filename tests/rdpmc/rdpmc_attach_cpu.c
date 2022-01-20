/* Does rdpmc give an error if you try to attach to the CPU? */

/* by Vince Weaver, vincent.weaver _at_ maine.edu       */

// why is this needed?
int sched_getcpu(void);


char test_string[]="Testing if rdpmc behavior on attach CPU...";
int quiet=0;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <sched.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#include "rdpmc_lib.h"

#include <sys/mman.h>

#define MAX_CPUS	16

int main(int argc, char **argv) {

	int i=0,j,result,num_cpus=1,current_cpu;
	long page_size=getpagesize();
	double error;

	void *addr[MAX_CPUS];

	struct perf_event_attr pe;

	int fd[MAX_CPUS],ret1,ret2;

	unsigned long long values[MAX_CPUS],enabled[MAX_CPUS],running[MAX_CPUS];

	quiet=test_quiet();

	if (!quiet) {
		printf("This tests rdpmc when attaching to CPU.\n");
	}

	/* See if we support rdpmc access */
	if (!detect_rdpmc(quiet)) {
		test_skip(test_string);
	}

	/* See if we support enough CPUs */
	// get_nprocs_conf(),
	num_cpus=get_nprocs();
	if (num_cpus<2) {
		if (!quiet) printf("Not running test, only %d cores found\n",num_cpus);
		test_skip(test_string);
	}

	if (!quiet) {
		printf("Found %d cpus\n\n",num_cpus);
	}

	/******************/
	/* Run on one CPU */
	/******************/

	if (!quiet) {
		printf("Trying rdpmc with 1 cpu, 100 million instructions\n\n");
	}

	/* Open event  */
	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	fd[0]=-1;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	fd[0]=perf_event_open(&pe,0,-1,fd[0],0);
	if (fd[0]<0) {
		fprintf(stderr,"Error opening event %d\n",0);
		test_fail(test_string);
	}

	/* mmap() event */
	addr[0]=mmap(NULL,page_size, PROT_READ, MAP_SHARED,fd[0],0);
	if (addr[0] == (void *)(-1)) {
		fprintf(stderr,"Error mmap()ing event %d!\n",0);
		test_fail(test_string);
	}

	/* start the event */
	ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);
	if (ret1<0) {
		fprintf(stderr,"Error enabling!\n");
	}

	for(j=0;j<100;j++) {
		result=instructions_million();
	}

	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

	/* read */
	values[0] = mmap_read_self(addr[0], &enabled[0], &running[0]);

	/* stop */
	ret2=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	if (ret1<0) {
		fprintf(stderr,"Error starting!\n");
		test_fail(test_string);
	}

	if (ret2<0) {
		fprintf(stderr,"Error stopping!\n");
		test_fail(test_string);
	}

	if (values[0]<0) {
		if (!quiet) printf("rdpmc support not available.\n");
		test_yellow_no(test_string);
	}

	if (!quiet) {
		printf("\tEvent %x -- Raw count: %lld "
				"enabled: %lld running: %lld\n",
				0,values[0],enabled[0],running[0]);
	}

	/* Close */
	close(fd[0]);
	munmap(addr[0],page_size);

	error=display_error(values[0], // avg
                                values[0],    // hi
                                values[0],    // low
                                100000000ULL,quiet);

        if (error_out_of_range(error)) {
                if (!quiet) printf("Error out of range!\n");
                test_fail(test_string);
        }


	/*******************/
	/* Run on all CPUs */
	/*******************/


	if (num_cpus>MAX_CPUS) num_cpus=MAX_CPUS;

	if (!quiet) {
		printf("\nTrying rdpmc with %d cpus, "
			"100 million instructions\n\n",num_cpus);
	}

	current_cpu=sched_getcpu();
	printf("Running on CPU %d\n",current_cpu);

	/* Open events  */
	for(i=0;i<num_cpus;i++) {
		memset(&pe,0,sizeof(struct perf_event_attr));

		pe.type=PERF_TYPE_HARDWARE;
		pe.size=sizeof(struct perf_event_attr);
		fd[i]=-1;
		pe.config=PERF_COUNT_HW_INSTRUCTIONS;
		pe.disabled=1;
		fd[i]=perf_event_open(&pe,0,i,-1,0);
		if (fd[i]<0) {
			fprintf(stderr,"Error opening event %d\n",i);
			test_fail(test_string);
		}

		/* mmap() event */
		addr[i]=mmap(NULL,page_size, PROT_READ, MAP_SHARED,fd[i],0);
		if (addr[i] == (void *)(-1)) {
			fprintf(stderr,"Error mmap()ing event %d!\n",i);
			test_fail(test_string);
		}
	}

	for(i=0;i<num_cpus;i++) {

		/* start the event */
		ret1=ioctl(fd[i], PERF_EVENT_IOC_ENABLE,0);
		if (ret1<0) {
			fprintf(stderr,"Error enabling!\n");
		}
	}

	for(j=0;j<100;j++) {
		result=instructions_million();
	}

	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");


	/* read */
	for(i=0;i<num_cpus;i++) {
		values[i] = mmap_read_self(addr[i], &enabled[i], &running[i]);
	}

	/* stop */
	for(i=0;i<num_cpus;i++) {
		ret2=ioctl(fd[i], PERF_EVENT_IOC_DISABLE,0);

		if (ret1<0) {
			fprintf(stderr,"Error starting!\n");
			test_fail(test_string);
		}

		if (ret2<0) {
			fprintf(stderr,"Error stopping!\n");
			test_fail(test_string);
		}

		if (values[i]<0) {
			if (!quiet) printf("rdpmc support not available.\n");
			test_yellow_no(test_string);
		}

		if (!quiet) {
			if (values[i]==-1) {
				printf("\tCPU %x -- rdpmc not available\n",i);
			}
			else {
				printf("\tCPU %x -- Raw count: %lld "
					"enabled: %lld running: %lld\n",
					i,values[i],enabled[i],running[i]);
			}
		}
	}

	/* Close */
	for(i=0;i<num_cpus;i++) {
		close(fd[i]);
		munmap(addr[i],page_size);
	}

	/* Validate */
	for(i=0;i<num_cpus;i++) {

		if (i!=current_cpu) {
			if (values[i]!=-1) {
				if (!quiet) {
					fprintf(stderr,"Error: CPU %d unexpectedly worked!\n",
						i);
				}
				test_fail(test_string);
			}
		}
		else {
			error=display_error(values[i], // avg
       		                        values[i],    // hi
              		                values[i],    // low
                       		        100000000ULL,quiet);

			if (error_out_of_range(error)) {
                		if (!quiet) printf("Error out of range!\n");
                		test_fail(test_string);
 		       }
		}
	}

	test_pass(test_string);

	return 0;
}
