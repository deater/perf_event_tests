char test_string[]="Testing if rdpmc works with event groups+syswide...";
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


#define MAX_EVENTS 16


int main(int argc, char **argv) {

	struct perf_counts_values counts[MAX_EVENTS] = {{ .val=0 },};
	struct perf_counts_values counts2[MAX_EVENTS] = {{ .val=0 },};
	struct perf_thread_map *threads, *threads_syswide;
	struct perf_evlist *evlist, *evlist_syswide;
	struct perf_evsel *evsel;
	struct perf_event_attr pe, syswide;
	struct perf_event_mmap_page *addr[MAX_EVENTS];
	unsigned int rdpmc_available[MAX_EVENTS];
	long long start_before,stop_after;
	int err,i,result,count=3;
	double error;


	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if rdpmc works with event groups.\n");
		printf("and if a syswide event is happening at the same time.\n\n");
	}

	/* See if we support rdpmc access */
	if (!detect_rdpmc(quiet)) {
		test_skip(test_string);
	}


	/*****************************/
	/* TEST START/WORK/READ/STOP */
	/*****************************/

	/* Open event  */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;

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

	for (i=0;i<count;i++) {
		if (i==0) {
			pe.disabled=1;
		}
		else {
			pe.disabled=0;
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
	i=0;
	perf_evlist__for_each_evsel(evlist, evsel) {
		err = perf_evsel__mmap(evsel, 0);
		if (err) {
			fprintf(stderr,"Error mmap()ing event %d!\n", i);
			test_fail(test_string);
		}
		i++;
	}


	/* Open syswide event  */
	memset(&syswide,0,sizeof(struct perf_event_attr));
	syswide.type=PERF_TYPE_HARDWARE;
	syswide.size=sizeof(struct perf_event_attr);
	syswide.config=PERF_COUNT_HW_INSTRUCTIONS;
	syswide.disabled=0;

	threads_syswide = perf_thread_map__new_dummy();
	if (!threads_syswide) {
		fprintf(stderr, "Error perf_thread_map__new_dummy\n");
		test_fail(test_string);
	}
	perf_thread_map__set_pid(threads_syswide, 0, 0);

	evlist_syswide = perf_evlist__new();
	if (!evlist_syswide) {
		fprintf(stderr, "Error perf_evlist__new\n");
		test_fail(test_string);
	}

	for (i=0;i<4;i++) {
		evsel = perf_evsel__new(&syswide);
		if (!evsel) {
			fprintf(stderr, "Error perf_evsel__new\n");
			test_fail(test_string);
		}

		perf_evlist__add(evlist_syswide, evsel);
	}

	perf_evlist__set_maps(evlist_syswide, NULL, threads_syswide);

	err = perf_evlist__open(evlist_syswide);
	if (err) {
		fprintf(stderr,"Error opening syswide event %d\n",err);
		test_fail(test_string);
	}
	perf_evlist__enable(evlist_syswide);


	/* start the rest */
	start_before=rdtsc();
	perf_evlist__enable(evlist);

	/* Check if RDPMC support is available */
	i=0;
	perf_evlist__for_each_evsel(evlist, evsel) {
		addr[i] = perf_evsel__mmap_base(evsel, 0, 0);
		rdpmc_available[i] = (addr[i]->cap_user_rdpmc && addr[i]->index);

		if (rdpmc_available[i] == 0) {
			fprintf(stderr,"\t  Event %d rdpmc support not available (cap_user_rdpmc = %d, index = %d)\n",
				i, addr[i]->cap_user_rdpmc, addr[i]->index);
			test_yellow_no(test_string);
		}
		i++;
	}


	for(i=0;i<100;i++) {
		result=instructions_million();
	}


	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");


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

	for(i=0;i<100;i++) {
		result=instructions_million();
	}

	/* read */
	i=0;
	perf_evlist__for_each_evsel(evlist_syswide, evsel) {
		err = perf_evsel__read(evsel, 0, 0, &counts2[i]);
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
	stop_after=rdtsc();


	if (!quiet) {
		printf("total start/read/stop latency: %lld cycles\n",
			stop_after-start_before);
		for (i=0;i<count;i++) {
			printf("\tEvent %x -- Raw count: %ld enabled: %ld running: %ld\n",
				i, counts[i].val, counts[i].ena, counts[i].run);
			printf("\t%ld\n",counts[i].val*counts[i].ena/counts[i].run);
			printf("\tEvent %x -- Raw count: %ld enabled: %ld running: %ld\n",
				i,counts2[i].val, counts2[i].ena, counts2[i].run);
		}
	}

	if (!quiet) printf("\n");
	error=display_error(counts[0].val, // avg
				counts[0].val,    // hi
				counts[0].val,    // low
				100000000ULL,quiet);

	if ((error>1.0) || ( error<-1.0)) {
		if (!quiet) printf("Error out of range!\n");
		test_fail(test_string);
	}

	error=display_error(counts2[0].val, // avg
				counts2[0].val,    // hi
				counts2[0].val,    // low
				200000000ULL,quiet);

	if ((error>1.0) || ( error<-1.0)) {
		if (!quiet) printf("Error out of range!\n");
		test_fail(test_string);
	}

	/* stop */
	perf_evlist__disable(evlist_syswide);


	/* Close */
	perf_evlist__for_each_evsel(evlist, evsel) {
		perf_evsel__munmap(evsel);
	}
	perf_evlist__close(evlist);
	perf_evlist__delete(evlist);
	perf_thread_map__put(threads);

	perf_evlist__close(evlist_syswide);
	perf_evlist__delete(evlist_syswide);
	perf_thread_map__put(threads_syswide);


	test_pass(test_string);

	return 0;
}
