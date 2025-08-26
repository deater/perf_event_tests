static char test_string[]="Testing if rdpmc with pthreads works...";
static int quiet=0;

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

#include <pthread.h>

#define MAX_EVENTS 16


struct thread_params {
	int num;
	int repeats;
};

static void *our_thread(void *arg) {

	struct perf_counts_values counts[MAX_EVENTS] = {{ .val=0 },};
	struct perf_thread_map *threads;
	struct perf_evlist *evlist;
	struct perf_evsel *evsel;
	struct perf_event_attr pe;
	struct perf_event_mmap_page *addr[MAX_EVENTS];
	unsigned int rdpmc_available[MAX_EVENTS];
	int err, count=4, i, result=0;

	struct thread_params *p = (struct thread_params *)arg;

	/*****************************/
	/* TEST START/WORK/READ/STOP */
	/*****************************/

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

	for(i=0;i<count;i++) {
		evsel = perf_evsel__new(&pe);
		if (!evsel) {
			fprintf(stderr, "Error perf_evsel__new\n");
			test_fail(test_string);
		}

		perf_evlist__add(evlist, evsel);
	}

	perf_evlist__set_maps(evlist, NULL, threads);

	err = perf_evlist__open(evlist);
	if (err) {
		fprintf(stderr,"Error opening event %d\n",err);
		test_fail(test_string);
	}

	/* mmap() event */
	perf_evlist__for_each_evsel(evlist, evsel) {
		err = perf_evsel__mmap(evsel, 0);
		if (err) {
			fprintf(stderr, "Error mmap()ing even %d!\n", 0);
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
			fprintf(stderr,"\t  Event %d rdpmc support not available (cap_user_rdpmc = %d, index = %d)\n",
				i, addr[i]->cap_user_rdpmc, addr[i]->index);
			test_yellow_no(test_string);
		}
		i++;
	}

	for(i=0;i<p->repeats;i++) {
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

	/* stop */
	perf_evlist__disable(evlist);

	perf_evlist__for_each_evsel(evlist, evsel) {
		perf_evsel__munmap(evsel);
	}
	perf_evlist__close(evlist);
	perf_evlist__delete(evlist);
	perf_thread_map__put(threads);

	/* Print results */
	if (!quiet) {
		for(i=0;i<count;i++) {
			if (rdpmc_available[i]!=1) {
				printf("Thread: %d Event: %d -- rdpmc not supported\n",p->num,i);
			}
			else {
				printf("Thread: %d Event: %d %lu\n",p->num,i,counts[i].val);
			}
		}
	}

	return NULL;
}

#define NUM_THREADS 8

int main(int argc, char **argv) {

	int i;
	pthread_t threads[NUM_THREADS];
	struct thread_params p[NUM_THREADS+1];

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if rdpmc works with pthreads\n\n");
	}

	/* See if we support rdpmc access */
	if (!detect_rdpmc(quiet)) {
		test_skip(test_string);
	}

	/* create 4 threads */

	for(i=0;i<NUM_THREADS;i++) {
		p[i].num=i;
		p[i].repeats=i*1000;
		pthread_create( &threads[i], NULL, our_thread,
				(void *) &p[i] );
	}

	p[NUM_THREADS].num=99;
	p[NUM_THREADS].repeats=100;

	our_thread( (void *)&p[NUM_THREADS]);

	for(i=0;i<NUM_THREADS;i++) {
		pthread_join(threads[i],NULL);
	}

	test_pass(test_string);

	return 0;
}
