int sched_getcpu(void);


char test_string[]="Testing if rdpmc behavior on attach all procs on other CPU...";
int quiet=0;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <sched.h>

/* libperf includes */
#include <perf/evlist.h>
#include <perf/evsel.h>
#include <perf/cpumap.h>
#include <perf/threadmap.h>
#include <perf/mmap.h>
#include <perf/core.h>
#include <perf/event.h>

#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#include "rdpmc_lib.h"

#include <sys/mman.h>

#define MAX_CPUS	16

int main(int argc, char **argv) {

	struct perf_counts_values counts[MAX_CPUS] = {{ .val=0 },};
	struct perf_counts_values counts_read[MAX_CPUS] = {{ .val=0 },};
	struct perf_cpu_map *cpus;
	struct perf_evsel *evsel;
	struct perf_event_attr pe;
	struct perf_event_mmap_page *addr[MAX_CPUS];
	unsigned int rdpmc_available[MAX_CPUS];
	int cpu,idx,err,ret1,ret2;
	double error;

	int j,result,num_cpus=1,current_cpu;

	quiet=test_quiet();

	if (!quiet) {
		printf("This tests rdpmc when attaching to all procs on other CPU.\n");
	}

	/* See if we support rdpmc access */
	if (!detect_rdpmc(quiet)) {
		test_skip(test_string);
	}

	/* See if we support enough CPUs */
	num_cpus=get_nprocs();
	if (num_cpus<2) {
		if (!quiet) printf("Not running test, only %d cores found\n",num_cpus);
		test_skip(test_string);
	}

	if (!quiet) {
		printf("Found %d cpus\n",num_cpus);
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


	memset(&pe,0,sizeof(struct perf_event_attr));

	/* Open events  */
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;

	cpus = perf_cpu_map__new(NULL);
	if (!cpus) {
		fprintf(stderr, "Error perf_cpu_map__new\n");
		test_fail(test_string);
	}

	evsel = perf_evsel__new(&pe);
	if (!evsel) {
		fprintf(stderr, "Error perf_evsel__new\n");
		test_fail(test_string);
	}

	err = perf_evsel__open(evsel, cpus, NULL);
	if (err) {
		fprintf(stderr,"Error opening event %d\n",err);
		test_fail(test_string);
	}

	/* mmap() event */
	err = perf_evsel__mmap(evsel, 0);
	if (err) {
		fprintf(stderr, "Error mmap()ing evsel\n");
		test_fail(test_string);
	}

	/* start the event */
	ret1 = perf_evsel__enable(evsel);
	if (ret1<0) {
		fprintf(stderr,"Error enabling!\n");
	}

	/* Check if RDPMC support is available */
	perf_cpu_map__for_each_cpu(cpu, idx, cpus) {
		addr[idx] = perf_evsel__mmap_base(evsel, idx, 0);
		rdpmc_available[idx] = (addr[idx]->cap_user_rdpmc && addr[idx]->index);
		if (rdpmc_available[idx] == 0) {
			if (!quiet) {
				fprintf(stderr,"\t  Event %d rdpmc support not available (cap_user_rdpmc = %d, index = %d)\n",
					 idx, addr[idx]->cap_user_rdpmc, addr[idx]->index);
			}
		}
	}


	for(j=0;j<100;j++) {
		result=instructions_million();
	}

	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

	/* read */
	perf_cpu_map__for_each_cpu(cpu, idx, cpus) {
		if (rdpmc_available[idx]==1) {
			err=perf_evsel__read(evsel, idx, 0, &counts[idx]);
		}
		else {
			err=perf_evsel__read(evsel, idx, 0, &counts_read[idx]);
		}
		if (err) {
			if (!quiet) {
				printf("Trouble reading event %d\n",idx);
				test_fail(test_string);
			}
		}
	}

	/* stop */
	ret2 = perf_evsel__disable(evsel);

	if (ret2<0) {
		fprintf(stderr,"Error stopping!\n");
		test_fail(test_string);
	}

	perf_cpu_map__for_each_cpu(cpu, idx, cpus) {
		if (!quiet) {
			if (rdpmc_available[idx] != 1) {
				printf("\tCPU %d -- rdpmc not available\n", idx);
				printf("\tCPU %d -- read() count: %lu\n", idx, counts_read[idx].val);
			}
			else {
				printf("\tCPU %d -- RDPMC count: %lu enabled: %lu running: %lu\n",
					idx, counts[idx].val, counts[idx].ena, counts[idx].run);
				printf("\tCPU %d -- read() count: %lu\n", idx, counts[idx].val);
			}
		}
	}

	/* Validate */
	perf_cpu_map__for_each_cpu(cpu, idx, cpus) {
		if (cpu!=current_cpu) {
			if (rdpmc_available[idx]==1) {
				if (!quiet) {
					fprintf(stderr,"Error: CPU %d unexpectedly worked!\n",
						cpu);
				}
				test_fail(test_string);
			}
		}
		else {
			error=display_error(counts[idx].val, // avg
					counts[idx].val,     // hi
					counts[idx].val,     // low
					100000000ULL,quiet);

			if ((error>1.0) || ( error<-1.0)) {
				if (!quiet) printf("Error out of range!\n");
				test_fail(test_string);
			}
		}
	}

	/* Close */
	perf_evsel__munmap(evsel);
	perf_evsel__close(evsel);
	perf_evsel__delete(evsel);
	perf_cpu_map__put(cpus);

	test_pass(test_string);

	return 0;
}
