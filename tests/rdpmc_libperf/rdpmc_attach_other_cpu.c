// why is this needed?
int sched_getcpu(void);


char test_string[]="Testing if rdpmc behavior on attach other CPU...";
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

#define MAX_CPUS	48

int main(int argc, char **argv) {

	struct perf_counts_values counts[MAX_CPUS] = {{ .val=0 },};
	struct perf_thread_map *threads;
	struct perf_cpu_map *cpus;
	struct perf_evlist *evlist;
	struct perf_evsel *evsel;
	struct perf_event_attr pe;
	struct perf_event_mmap_page *addr[MAX_CPUS];
	unsigned int rdpmc_available[MAX_CPUS];
	int cpu,idx,err;
	int j,result,num_cpus=1,current_cpu,running_cpu;
	double error;

	pid_t pid;

	quiet=test_quiet();

	if (!quiet) {
		printf("This tests rdpmc when attaching to other CPU.\n");
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
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;

	threads = perf_thread_map__new_dummy();
	if (!threads) {
		fprintf(stderr, "Error perf_thread_map__new_dummy\n");
		test_fail(test_string);
	}
	perf_thread_map__set_pid(threads, 0, 0);

	evlist = perf_evlist__new();
	if (!evlist) {
		fprintf(stderr, "Error perf_evlist__new\n");
		test_fail(test_string);
	}

	evsel = perf_evsel__new(&pe);
	if (!evsel) {
		fprintf(stderr, "Error perf_evsel__new\n");
		test_fail(test_string);
	}

	perf_evlist__add(evlist, evsel);
	perf_evlist__set_leader(evlist);
	perf_evlist__set_maps(evlist, NULL, threads);

	err = perf_evlist__open(evlist);
	if (err) {
		fprintf(stderr,"Error opening event\n");
		test_fail(test_string);
	}

	/* mmap event */
	perf_evlist__for_each_evsel(evlist, evsel) {
		err = perf_evsel__mmap(evsel, 0);
		if (err) {
			fprintf(stderr,"\tError mmap()ing event!\n");
			test_fail(test_string);
		}
	}

	/* start the event */
	perf_evlist__enable(evlist);

	/* Check if RDPMC support is available */
	idx=0;
	perf_evlist__for_each_evsel(evlist, evsel) {
		addr[idx] = perf_evsel__mmap_base(evsel, 0, 0);
		rdpmc_available[idx] = (addr[idx]->cap_user_rdpmc && addr[idx]->index);

		if (rdpmc_available[idx] == 0) {
			if (!quiet) {
				fprintf(stderr,"\t  Event %d rdpmc support not available (cap_user_rdpmc = %d, index = %d)\n",
					idx, addr[idx]->cap_user_rdpmc, addr[idx]->index);
			}
		}
		idx++;
	}

	/* test */
	for(j=0;j<100;j++) {
		result=instructions_million();
	}
	if (result==CODE_UNIMPLEMENTED)
		printf("Warning, no million\n");

	/* read */
	idx=0;
	perf_evlist__for_each_evsel(evlist, evsel) {
		err = perf_evsel__read(evsel, idx, 0, &counts[idx]);
		if (err) {
			if (!quiet) {
				printf("Trouble reading event %d\n", 0);
				test_fail(test_string);
			}
		}
		idx++;
	}

	/* stop */
	perf_evlist__disable(evlist);

	if (!quiet) {
		printf("\tEvent %d -- Raw count: %ld enabled: %ld running: %ld\n",
				0, counts[0].val, counts[0].ena, counts[0].run);
	}

	/* Close */
	perf_evlist__for_each_evsel(evlist, evsel) {
		perf_evsel__munmap(evsel);
	}
	perf_evlist__close(evlist);
	perf_evlist__delete(evlist);

	error=display_error(counts[0].val, // avg
			counts[0].val,    // hi
			counts[0].val,    // low
			100000000ULL,quiet);

	if ((error>1.0) || ( error<-1.0)) {
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
	printf("Measuring on CPU %d\n",current_cpu);

	/* Open events  */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;

	threads = perf_thread_map__new_dummy();
	if (!threads) {
		fprintf(stderr, "Error perf_thread_map__new_dummy\n");
		test_fail(test_string);
	}
	perf_thread_map__set_pid(threads, 0, 0);

	cpus = perf_cpu_map__new(NULL);
	if (!cpus) {
		fprintf(stderr, "Error perf_cpu_map__new\n");
		test_fail(test_string);
	}

	evlist = perf_evlist__new();
	if (!evlist) {
		fprintf(stderr, "Error perf_evlist__new\n");
		test_fail(test_string);
	}

	evsel = perf_evsel__new(&pe);
	if (!evsel) {
		fprintf(stderr, "Error perf_evsel__new\n");
		test_fail(test_string);
	}

	perf_evlist__add(evlist, evsel);
	perf_evlist__set_maps(evlist, cpus, threads);

	err = perf_evlist__open(evlist);
	if (err) {
		fprintf(stderr, "Error opening event\n");
		test_fail(test_string);
	}

	/* mmap event */
	perf_evlist__for_each_evsel(evlist, evsel) {
		err = perf_evsel__mmap(evsel, 0);
		if (err) {
			fprintf(stderr, "\tError mmap()ing event!\n");
			test_fail(test_string);
		}
	}

	/* start */
	perf_evlist__enable(evlist);

	/* Check if RDPMC support is available */
	perf_evlist__for_each_evsel(evlist, evsel) {
		cpus = perf_evsel__cpus(evsel);

		perf_cpu_map__for_each_cpu(cpu, idx, cpus) {
			addr[idx] = perf_evsel__mmap_base(evsel, cpu, 0);
			rdpmc_available[idx] = (addr[idx]->cap_user_rdpmc && addr[idx]->index);

			if (rdpmc_available[idx] == 0) {
				if (!quiet) {
					fprintf(stderr,"\t  EVENT %d rdpmc support not available (cap_user_rdpmc = %d, index = %d)\n",
						idx, addr[idx]->cap_user_rdpmc, addr[idx]->index);
				}
			}
		}
	}


	/* test */
	pid=fork();
	if (pid==0) {

		running_cpu=sched_getcpu();
		printf("Running on CPU %d\n",running_cpu);

		for(j=0;j<100;j++) {
			result=instructions_million();
		}
		if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");
		exit(0);
	}
	sleep(1);


	/* read */
	idx=0;
	perf_evlist__for_each_evsel(evlist, evsel) {
		cpus = perf_evsel__cpus(evsel);

		perf_cpu_map__for_each_cpu(cpu, idx, cpus) {
			err = perf_evsel__read(evsel, idx, 0, &counts[idx]);
			if (err) {
				if (!quiet) {
					printf("Trouble reading event %d\n", 0);
					test_fail(test_string);
				}
			}
		}
	}


	/* stop */
	perf_evlist__disable(evlist);


	idx=0;
	perf_evlist__for_each_evsel(evlist, evsel) {
		cpus = perf_evsel__cpus(evsel);

		perf_cpu_map__for_each_cpu(cpu, idx, cpus) {
			if (!quiet) {
				if (rdpmc_available[idx] != 1) {
					printf("\tCPU %d -- rdpmc not available\n",cpu);
				}
				else {
					printf("\tCPU %d -- Raw count: %lu enabled: %lu running: %lu\n",
						cpu, counts[idx].val, counts[idx].ena, counts[idx].run);
				}
			}
		}
	}


	/* Validate */
	perf_evlist__for_each_evsel(evlist, evsel) {
		cpus = perf_evsel__cpus(evsel);

		perf_cpu_map__for_each_cpu(cpu, idx, cpus) {
			if (cpu != current_cpu) {
				if (rdpmc_available[idx] == 1) {
					if (!quiet) {
						fprintf(stderr,"Error: CPU %d not 0!\n", idx);
					}
					test_fail(test_string);
				}
			}
			else {
				if (counts[idx].val>900000000) {
					if (!quiet) printf("Should not be this high.\n");
					test_fail(test_string);
				}
			}
		}
	}

	/* Close */
	perf_evlist__for_each_evsel(evlist, evsel) {
		perf_evsel__munmap(evsel);
	}
	perf_evlist__close(evlist);
	perf_evlist__delete(evlist);
	perf_cpu_map__put(cpus);
	perf_thread_map__put(threads);

	test_pass(test_string);

	return 0;
}
