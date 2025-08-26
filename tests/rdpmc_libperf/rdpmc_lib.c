/* This file contains rdpmc access helper functions            */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
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

#include "../rdpmc/rdpmc_inlines.h"
#include "rdpmc_lib.h"

#include <sys/mman.h>

#define MAX_EVENTS 16

static int libperf_print(enum libperf_print_level level,
			 const char *fmt, va_list ap)
{
	return vfprintf(stderr, fmt, ap);
}

int detect_rdpmc(int quiet)
{
	struct perf_thread_map *threads;
	struct perf_evsel *evsel;
	struct perf_event_attr pe;
	struct perf_event_mmap_page *addr;
	int err, ret = 0;

	libperf_init(libperf_print);

	memset(&pe, 0, sizeof(struct perf_event_attr));

	pe.type = PERF_TYPE_HARDWARE;
	pe.size = sizeof(struct perf_event_attr);
	pe.config = PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled = 1;

	threads = perf_thread_map__new_dummy();
	if (!threads) {
		fprintf(stderr, "Error perf_thread_map__new_dummy\n");
		return 0;
	}
	perf_thread_map__set_pid(threads, 0, 0);

	evsel = perf_evsel__new(&pe);
	if (!evsel) {
		fprintf(stderr, "Error perf_evsel__new\n");
		goto out_thread;
	}

	err = perf_evsel__open(evsel, NULL, threads);
	if (err) {
		fprintf(stderr, "Error opening event %d\n", err);
		goto out_open;
	}

	err = perf_evsel__mmap(evsel, 0);
	if (err) {
		fprintf(stderr, "Error mmap()ing event\n");
		goto out_mmap;
	}

	addr = perf_evsel__mmap_base(evsel, 0, 0);
	if (addr->cap_user_rdpmc) {
		/* new 3.12 and newer rdpmc support found */
		ret = 1;
		goto out_mmap;
	} else {
		if (addr->cap_bit0_is_deprecated && addr->cap_bit0) {
			/* 3.4 to 3.11 broken support */
			ret = 2;
			goto out_mmap;
		}
	}

	if (!quiet) {
		printf("rdpmc support not detected (mmap->cap_user_rdpmc==%d)\n",
		       addr->cap_user_rdpmc);
	}

out_mmap:
	perf_evsel__munmap(evsel);
	perf_evsel__close(evsel);

out_open:
	perf_evsel__delete(evsel);

out_thread:
	perf_thread_map__put(threads);

	return ret;
}
