static char test_string[]="Testing if minimized rdpmc papi-multi-attach works...";
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

#define MAX_EVENTS 16


static int wait_for_attach_and_loop( int num ) {

	kill( getpid(), SIGSTOP );

	kill( getpid(), SIGSTOP );

	return 0;
}


int main(int argc, char **argv) {

	struct perf_counts_values counts = {{ .val=0 },};
	struct perf_thread_map *threads;
	struct perf_evsel *evsel;
	struct perf_event_attr pe;
	struct perf_event_mmap_page *addr;
	unsigned int rdpmc_available;
	long long start_before,stop_after;
	int ret1,ret2,err, status,i;

	pid_t pid;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if multiple enable cycles on attach works.\n\n");
	}

	/* See if we support rdpmc access */
	if (!detect_rdpmc(quiet)) {
		test_skip(test_string);
	}

	/*********************************/
	/* start and pause two children  */
	/*********************************/

	/* fork a child */
	pid = fork();
	if ( pid < 0 ) {
		fprintf(stderr,"Failed fork\n");
		test_fail(test_string);
	}
	else if (pid==0) {
		/* in child */
		exit(wait_for_attach_and_loop(1));
	}

	if ( ptrace( PTRACE_ATTACH, pid, NULL, NULL ) == -1 ) {
		fprintf(stderr,"Error attaching to %d\n",pid);
		return -1;
	}
	if ( waitpid( pid, &status, 0 ) == -1 ) {
		fprintf(stderr,"Error waitpid %d\n",pid);
		return -1;
	}
	if ( WIFSTOPPED( status ) == 0 ) {
		fprintf(stderr,"WIFSTOPPED didn't happen %d\n",pid);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("* We have opened a child: %d\n\n",pid);
	}


	/*****************************/
	/* TEST START/WORK/READ/STOP */
	/*****************************/

	/* Open event, pid  */
	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);

	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	/* pinned doesn't seem to matter */
	pe.pinned=1;

	threads = perf_thread_map__new_dummy();
	if (!threads) {
		fprintf(stderr, "Error perf_thread_map__new_dummy\n");
		test_fail(test_string);
	}
	perf_thread_map__set_pid(threads, 0, pid);

	evsel = perf_evsel__new(&pe);
	if (!evsel) {
		fprintf(stderr, "Error perf_evsel__new\n");
		test_fail(test_string);
	}

	err = perf_evsel__open(evsel, NULL, threads);
	if (err) {
		fprintf(stderr,"Error opening event\n");
		test_fail(test_string);
	}

	/* mmap() event */
	err = perf_evsel__mmap(evsel, 0);
	if (err) {
		fprintf(stderr,"Error mmap()ing event!\n");
		test_fail(test_string);
	}

	if (!quiet) printf("+ We've attached one event to %d\n",pid);


	/* Magic */
	for(i=0;i<2;i++) {
		ret1=perf_evsel__enable(evsel);
		ret1=perf_evsel__disable(evsel);
	}

	/* start */
	start_before=rdtsc();

	ret1=perf_evsel__enable(evsel);

	/* Check if RDPMC support is available */
	addr = perf_evsel__mmap_base(evsel, 0, 0);
	rdpmc_available = (addr->cap_user_rdpmc && addr->index);
	if (rdpmc_available == 0) {
		if (!quiet) {
			fprintf(stderr,"\t  Event %d rdpmc support not available (cap_user_rdpmc = %d, index = %d)\n",
				0, addr->cap_user_rdpmc, addr->index);
		}

		test_yellow_no(test_string);
	}

	/* start up pid */
	if ( ptrace( PTRACE_CONT, pid, NULL, NULL ) == -1 ) {
		fprintf(stderr,"Couldn't continue %d\n",pid);
		return -1;
	}
	if ( waitpid( pid, &status, 0 ) == -1 ) {
		fprintf(stderr,"Couldn't waitpid() %d\n",pid );
		return -1;
	}
	if ( WIFSTOPPED( status ) == 0 ) {
		test_fail(test_string);
	}
	if ( WSTOPSIG( status ) != SIGSTOP ) {
		test_fail(test_string);
	}

	/* Wait for the SIGSTOP. */
	if ( ptrace( PTRACE_CONT, pid, NULL, NULL ) == -1 ) {
		fprintf(stderr, "sigstop fail\n" );
		return -1;
	}

	if ( waitpid( pid, &status, 0 ) == -1 ) {
		fprintf(stderr, "waitpid() pid\n" );
		return -1;
	}
	if ( WIFSTOPPED( status ) == 0 ) {
		test_fail(test_string);
	}
	if ( WSTOPSIG( status ) != SIGSTOP ) {
		test_fail(test_string);
	}

	/* read */
	err = perf_evsel__read(evsel, 0, 0, &counts);
	if (err) {
		if (!quiet) {
			printf("Trouble reading event %d\n", 0);
			test_fail(test_string);
		}
	}

	/* stop */
	ret2 = perf_evsel__disable(evsel);

	stop_after=rdtsc();

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
		printf("Trying attach: %lld cycles\n",
			stop_after-start_before);
		printf("\t* RDPMC 1 Event %d -- count: %ld enabled %ld running: %ld\n",
				0,counts.val,counts.ena,counts.run);
	}

	if (counts.ena!=counts.run) {
		if (!quiet) printf("enabled doesn't match running!\n");
		if (check_linux_version_newer(4,14,0)) {
			test_fail(test_string);
		}
		else {
			test_known_kernel_bug(test_string);
		}
	}

	test_pass(test_string);

	return 0;
}
