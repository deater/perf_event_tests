char test_string[]="Testing if rdpmc fallback works on sw events...";
int quiet=0;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

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
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <signal.h>

#define MAX_EVENTS 16

int main(int argc, char **argv) {

	struct perf_counts_values counts[MAX_EVENTS] = {{ .val=0 },};
	struct perf_thread_map *threads;
	struct perf_evlist *evlist;
	struct perf_evsel *evsel;
	struct perf_event_attr pe;
	struct perf_event_mmap_page *addr[MAX_EVENTS];
	unsigned int rdpmc_available[MAX_EVENTS];
	int err,i,result,count=2;
	double error;


	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if rdpmc with sw events falls back.\n\n");
	}

	/* See if we support rdpmc access */
	if (!detect_rdpmc(quiet)) {
		test_skip(test_string);
	}

	/*****************************/
	/* TEST START/WORK/READ/STOP */
	/*****************************/

	/* Open event, pid1  */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.size=sizeof(struct perf_event_attr);
	pe.read_format=PERF_FORMAT_TOTAL_TIME_ENABLED |
			PERF_FORMAT_TOTAL_TIME_RUNNING;

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

	for(i=0;i<count;i++) {
		if (i==0) {
			pe.type=PERF_TYPE_HARDWARE;
			pe.config=PERF_COUNT_HW_INSTRUCTIONS;
			pe.disabled=1;
			pe.pinned=1;
		}
		else {
			pe.type=PERF_TYPE_SOFTWARE;
			pe.config=PERF_COUNT_SW_TASK_CLOCK;
			pe.disabled=0;
			pe.pinned=0;
		}

		evsel = perf_evsel__new(&pe);
		if (!evsel) {
			fprintf(stderr, "Error perf_evsel__new\n");
			test_fail(test_string);
		}

		perf_evlist__add(evlist, evsel);
	}

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
			fprintf(stderr,"Error mmap()ing event %d!\n",i);
			test_fail(test_string);
		}
	}


	/* start */
	perf_evlist__enable(evlist);


	/* Check if RDPMC support is available */
	i=0;
	perf_evlist__for_each_evsel(evlist, evsel) {
		addr[i] = perf_evsel__mmap_base(evsel, 0, 0);
		rdpmc_available[i] = (addr[i]->cap_user_rdpmc && addr[i]->index);
		if (rdpmc_available[i] == 0) {
			if (!quiet) {
				fprintf(stderr,"\t  Event %d rdpmc support not available (cap_user_rdpmc = %d, index = %d)\n",
					i, addr[i]->cap_user_rdpmc, addr[i]->index);
			}
		}
		i++;
	}


	result=instructions_million();
	if (result==CODE_UNIMPLEMENTED) {
		test_skip(test_string);
	}

	/* read */
	i=0;
	perf_evlist__for_each_evsel(evlist, evsel) {
		err = perf_evsel__read(evsel, 0, 0, &counts[i]);
		if (err) {
			if (!quiet) {
				printf("Trouble reading event %d\n", 0);
				test_fail(test_string);
			}
		}
		i++;
	}


	/* stop */
	perf_evlist__disable(evlist);


	/* read */
	i=0;
	perf_evlist__for_each_evsel(evlist, evsel) {
		err = perf_evsel__read(evsel, 0, 0, &counts[i]);
		if (err) {
			if (!quiet) {
				printf("Trouble reading event %d\n", 0);
				test_fail(test_string);
			}
		}
		i++;
	}


	if (!quiet) {

		for(i=0;i<count;i++) {
			if (rdpmc_available[i]==1) {
				printf("\t* RDPMC 1 Event %x -- count: %lu enabled %lu running: %lu\n",
					i,counts[i].val,counts[i].ena,counts[i].run);
			}
			else {
				printf("\t* RDPMC 1 Event %x -- rdpmc support not available\n",i);
			}
		}

		for(i=0;i<count;i++) {
			printf("\t* READ() 1 Event %x -- count: %lu enabled %lu running: %lu\n",
				i,counts[i].val,counts[i].ena,counts[i].run);
		}

	}

	if (!quiet) printf("\n");

	error=display_error(counts[0].val,
				counts[0].val,
				counts[0].val,
				1000000ULL,quiet);

	if ((error>10.0) || ( error<-10.0)) {
		if (!quiet) printf("Error out of range!\n");
		test_fail(test_string);
	}

	perf_evlist__for_each_evsel(evlist, evsel) {
		perf_evsel__munmap(evsel);
	}
	perf_evlist__close(evlist);
	perf_evlist__delete(evlist);
	perf_thread_map__put(threads);

	test_pass(test_string);

	return 0;
}
