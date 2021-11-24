static char test_string[]="Testing if we can rdpmc in execed process...";
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

#include "rdpmc_lib.h"

#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <signal.h>


int main(int argc, char **argv) {

	struct perf_counts_values counts = {{ .val=0 },};
	struct perf_thread_map *threads;
	struct perf_evsel *evsel;
	struct perf_event_attr pe;
	struct perf_event_mmap_page *addr;
	unsigned int rdpmc_available;
	int ret1,ret2,err;

	int in_child=0,result;
	char current_dir[BUFSIZ];
	char exe_path[BUFSIZ];


	if (argc>1) in_child=1;

	getcwd(current_dir,BUFSIZ);

	if (argv[0][0]=='/') {
		snprintf(exe_path,BUFSIZ,"%s",argv[0]);
	}
	else {
		snprintf(exe_path,BUFSIZ,"%s/%s",current_dir,argv[0]);
	}

	quiet=test_quiet();

	if (!quiet) {
		if (!in_child) {
			printf("This test checks if rdpmc events work in exec()ed process.\n\n");
			printf("+ Before exec\n");
		}
		else {
			printf("+ After exec\n");
		}
	}


	/* See if we support rdpmc access */
	if (!detect_rdpmc(quiet)) {
		test_skip(test_string);
	}

	/* Open event */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.pinned=1;

	threads = perf_thread_map__new_dummy();
	if (!threads) {
		fprintf(stderr, "Error perf_thread_map__new_dummy\n");
		test_fail(test_string);
	}
	perf_thread_map__set_pid(threads, 0, 0);

	evsel = perf_evsel__new(&pe);
	if (!evsel) {
		fprintf(stderr, "Error perf_thread_map__new_dummy\n");
		test_fail(test_string);
	}

	err = perf_evsel__open(evsel, NULL, threads);
	if (err) {
		fprintf(stderr, "Error opening event %d\n",err);
		test_fail(test_string);
	}

	/* mmap() event */
	err = perf_evsel__mmap(evsel, 0);
	if (err) {
		fprintf(stderr, "Error mmap()ing event %d\n",err);
		test_fail(test_string);
	}

	/* start */
	ret1 = perf_evsel__enable(evsel);

	/* Check if RDPMC support is available */
	addr = perf_evsel__mmap_base(evsel, 0, 0);
	rdpmc_available = (addr->cap_user_rdpmc && addr->index);
	if (rdpmc_available == 0) {
		fprintf(stderr,"\t  Event %d rdpmc support not available (cap_user_rdpmc = %d, index = %d)\n",
			0, addr->cap_user_rdpmc, addr->index);
	}

	/* read */
	err = perf_evsel__read(evsel, 0, 0, &counts);
	if (err) {
		fprintf(stderr, "Trouble reading event %d\n",0);
	}

	/* stop */
	ret2 = perf_evsel__disable(evsel);

	if (ret1<0) {
		fprintf(stderr,"Error starting!\n");
		test_fail(test_string);
	}

	if (ret2<0) {
		fprintf(stderr,"Error stopping!\n");
		test_fail(test_string);
	}


	/* Close */
	perf_evsel__munmap(evsel);
	perf_evsel__close(evsel);
	perf_evsel__delete(evsel);
	perf_thread_map__put(threads);


	if (!quiet) {
		printf("\t* RDPMC 1 Event %x -- count: %lu enabled %lu running: %lu\n",
			0, counts.val, counts.ena, counts.run);
	}


	if (counts.ena!=counts.run) {
		if (!quiet) printf("enabled doesn't match running!\n");
		test_fail(test_string);
	}

	if (!in_child) {
		result=execl( exe_path, exe_path ,"1",NULL);
		if (result<0) {
			if (!quiet) printf("Failed exec!\n");
			test_fail(test_string);
		}
	}


	test_pass(test_string);

	return 0;
}
