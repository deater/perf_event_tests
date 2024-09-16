char test_string[] = "Testing if userspace rdpmc reads are supported...";
int quiet = 0;

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

#define MAX_EVENTS 16

int main(int argc, char **argv)
{
	struct perf_counts_values stamp[MAX_EVENTS] = {{ .val=0 },};
	struct perf_counts_values now[MAX_EVENTS] = {{ .val=0 },};
	struct perf_thread_map *threads;
	struct perf_evlist *evlist;
	struct perf_evsel *evsel;
	struct perf_event_attr pe;
	struct perf_event_mmap_page *addr[MAX_EVENTS];
	unsigned int rdpmc_available[MAX_EVENTS];
	long long start_before, stop_after;
	int err, i, rdpmc_support = 0, count = 1;

	quiet = test_quiet();

	if (!quiet)
		printf("This test checks if userspace rdpmc() style reads work.\n\n");


	/* detect support */

	rdpmc_support=detect_rdpmc(quiet);

	if (rdpmc_support==0)
		test_skip(test_string);

	/***************/
	/* SIMPLE TEST */
	/***************/

	/* open */
	memset(&pe, 0, sizeof(struct perf_event_attr));
	pe.type = PERF_TYPE_HARDWARE;
	pe.size = sizeof(struct perf_event_attr);
	pe.config = PERF_COUNT_HW_INSTRUCTIONS;

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

	for (i = 0; i < count; i++) {
		if (i == 0) {
			pe.disabled = 1;
			pe.pinned = 1;
		} else {
			pe.disabled = 0;
			pe.pinned = 0;
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
		fprintf(stderr, "Error opening event\n");
		test_fail(test_string);
	}

	/* mmap event */
	perf_evlist__for_each_evsel(evlist, evsel) {
		err = perf_evsel__mmap(evsel, 0);
		if (err) {
			fprintf(stderr, "Error mmap()ing event %d!\n", i);
			test_fail(test_string);
		}
	}

	/* start */
	start_before = rdtsc();
	perf_evlist__enable(evlist);

	/* Check if RDPMC support is available */
	i = 0;
	perf_evlist__for_each_evsel(evlist, evsel) {
		addr[i] = perf_evsel__mmap_base(evsel, 0, 0);
		rdpmc_available[i] = (addr[i]->cap_user_rdpmc && addr[i]->index);
		if (rdpmc_available[i] == 0) {
			fprintf(stderr, "\t  Event %d rdpmc support not available (cap_user_rdpmc = %d, index = %d)\n",
				i, addr[i]->cap_user_rdpmc, addr[i]->index);
			test_yellow_no(test_string);
		}
		i++;
	}

	/* read */
	i = 0;
	perf_evlist__for_each_evsel(evlist, evsel) {
		err = perf_evsel__read(evsel, 0, 0, &stamp[i]);
		err |= perf_evsel__read(evsel, 0, 0, &now[i]);
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

	stop_after = rdtsc();

	if (!quiet) {
		printf("total start/read/stop latency: %lld cycles\n",
		       stop_after - start_before);
		for (i = 0; i < count; i++) {
			printf("\tEvent %x -- count: %lu running: %lu\n",
			       i, now[i].val - stamp[i].val, now[i].run - stamp[i].run);
		}
	}

	perf_evlist__for_each_evsel(evlist, evsel) {
		perf_evsel__munmap(evsel);
	}
	perf_evlist__close(evlist);
	perf_evlist__delete(evlist);
	perf_thread_map__put(threads);

	if (rdpmc_support == 1) {
		if (!quiet)
			printf("New behavior\n");
	} else {
		test_yellow_old_behavior(test_string);
	}

	test_pass(test_string);

	return 0;
}
