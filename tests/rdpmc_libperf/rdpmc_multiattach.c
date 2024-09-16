char test_string[]="Testing if rdpmc multi-attach works...";
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


static int wait_for_attach_and_loop( int num ) {

	int i,result=0;

//	printf("BEFORE START\n");
	kill( getpid(), SIGSTOP );

//	printf("STARTING\n");
	for(i=0;i<num;i++) {
		result=instructions_million();
	}
//	sleep(5);

	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

//	printf("BEFORE STOP\n");
	kill( getpid(), SIGSTOP );
//	printf("EXITING\n");
	return 0;
}


int main(int argc, char **argv) {

	struct perf_counts_values counts[MAX_EVENTS] = {{ .val=0 },};
	struct perf_counts_values counts2[MAX_EVENTS] = {{ .val=0 },};
	struct perf_thread_map *threads, *threads2;
	struct perf_evlist *evlist, *evlist2;
	struct perf_evsel *evsel;
	struct perf_event_attr pe;
	struct perf_event_mmap_page *addr[MAX_EVENTS];
	struct perf_event_mmap_page *addr2[MAX_EVENTS];
	unsigned int rdpmc_available[MAX_EVENTS];
	unsigned int rdpmc_available2[MAX_EVENTS];
	int err,status,i,count=2;
	long long start_before,stop_after;
	double error;


	pid_t pid,pid2;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if rdpmc multi-attach works.\n\n");
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



	/* fork a second child */
	pid2 = fork();
	if ( pid2 < 0 ) {
		fprintf(stderr,"Failed fork\n");
		test_fail(test_string);
	}
	else if (pid2==0) {
		/* in child */
		exit(wait_for_attach_and_loop(2));
	}

	if ( ptrace( PTRACE_ATTACH, pid2, NULL, NULL ) == -1 ) {
		fprintf(stderr,"Error attaching to %d\n",pid2);
		return -1;
	}
	if ( waitpid( pid2, &status, 0 ) == -1 ) {
		fprintf(stderr,"Error waitpid %d\n",pid2);
		return -1;
	}
	if ( WIFSTOPPED( status ) == 0 ) {
		fprintf(stderr,"WIFSTOPPED didn't happen %d\n",pid2);
		test_fail(test_string);
	}



	/*****************************/
	/* TEST START/WORK/READ/STOP */
	/*****************************/

	/* Open event, pid1  */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);

	threads = perf_thread_map__new_dummy();
	if (!threads) {
		fprintf(stderr, "Error perf_thread_map__new_dummy\n");
		test_fail(test_string);
	}
	perf_thread_map__set_pid(threads, 0, pid);

	evlist = perf_evlist__new();
	if (!evlist) {
		fprintf(stderr, "Error perf_evlist__new\n");
		test_fail(test_string);
	}

	for(i=0;i<count;i++) {

		if (i==0) {
			pe.config=PERF_COUNT_HW_INSTRUCTIONS;
			pe.disabled=1;
			pe.pinned=1;
		}
		else {
			pe.config=PERF_COUNT_HW_CPU_CYCLES;
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


	/* open event */
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


	/* Open event, pid2  */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);

	threads2 = perf_thread_map__new_dummy();
	if (!threads2) {
		fprintf(stderr, "Error perf_thread_map__new_dummy\n");
		test_fail(test_string);
	}
	perf_thread_map__set_pid(threads2, 0, pid2);

	evlist2 = perf_evlist__new();
	if (!evlist2) {
		fprintf(stderr, "Error perf_evlist__new\n");
		test_fail(test_string);
	}

	for(i=0;i<count;i++) {
		if (i==0) {
			pe.config=PERF_COUNT_HW_INSTRUCTIONS;
			pe.disabled=1;
			pe.pinned=1;
		}
		else {
			pe.config=PERF_COUNT_HW_CPU_CYCLES;
			pe.disabled=0;
			pe.pinned=0;
		}

		evsel = perf_evsel__new(&pe);
		if (!evsel) {
			fprintf(stderr, "failed to create evsel\n");
			test_fail(test_string);
		}

		perf_evlist__add(evlist2, evsel);
	}

	perf_evlist__set_leader(evlist2);
	perf_evlist__set_maps(evlist2, NULL, threads2);

	err = perf_evlist__open(evlist2);
	if (err) {
		fprintf(stderr,"Error opening event\n");
		test_fail(test_string);
	}

	/* mmap event */
	perf_evlist__for_each_evsel(evlist2, evsel) {
		err = perf_evsel__mmap(evsel, 0);
		if (err) {
			fprintf(stderr,"Error mmap()ing event %d!\n",i);
			test_fail(test_string);
		}
	}


	/* start */
	start_before=rdtsc();
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


	perf_evlist__enable(evlist2);

	/* Check if RDPMC support is available */
	i=0;
	perf_evlist__for_each_evsel(evlist2, evsel) {
		addr2[i] = perf_evsel__mmap_base(evsel, 0, 0);
		rdpmc_available2[i] = (addr2[i]->cap_user_rdpmc && addr2[i]->index);
		if (rdpmc_available2[i] == 0) {
			if (!quiet) {
				fprintf(stderr,"\t  Event %d rdpmc support not available (cap_user_rdpmc = %d, index = %d)\n",
					i, addr2[i]->cap_user_rdpmc, addr2[i]->index);
			}
		}
		i++;
	}


	/* start up pid2 */
	if ( ptrace( PTRACE_CONT, pid2, NULL, NULL ) == -1 ) {
		fprintf(stderr,"Couldn't continue %d\n",pid2);
		return -1;
	}
	if ( waitpid( pid2, &status, 0 ) == -1 ) {
		fprintf(stderr,"Couldn't waitpid() %d\n",pid2 );
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


	/* Wait for the SIGSTOP. */
	if ( ptrace( PTRACE_CONT, pid2, NULL, NULL ) == -1 ) {
		fprintf(stderr, "sigstop fail\n" );
		return -1;
	}

	if ( waitpid( pid2, &status, 0 ) == -1 ) {
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
	perf_evlist__disable(evlist2);


	/* read */
	i=0;
	perf_evlist__for_each_evsel(evlist2, evsel) {
		err = perf_evsel__read(evsel, 0, 0, &counts2[i]);
		if (err) {
			if (!quiet) {
				printf("Trouble reading event %d\n", 0);
				test_fail(test_string);
			}
		}
		i++;
	}

	stop_after=rdtsc();


	if (!quiet) {
		printf("Trying attach: %lld cycles\n",
			stop_after-start_before);
		for(i=0;i<count;i++) {
			if (rdpmc_available[i]!=1) {
				printf("\t* RDPMC 1 Event %d -- rdpmc not supported\n",i);
			}
			else {
				printf("\t* RDPMC 1 Event %d -- count: %lu enabled %lu running: %lu\n",
					i,counts[i].val,counts[i].ena,counts[i].run);
			}
		}
		for(i=0;i<count;i++) {
			if (rdpmc_available2[i]!=1) {
				printf("\t* RDPMC 2 Event %d -- rdpmc not supported\n",i);
			}
			else {
				printf("\t* RDPMC 2 Event %d -- count: %lu enabled %lu running: %lu\n",
					i,counts2[i].val,counts2[i].ena,counts2[i].run);
			}
		}
	}


	if (!quiet) printf("\n");

	if ((rdpmc_available[0]==1) || (rdpmc_available2[0]==1)) {

		error=display_error(counts[0].val,
					counts[0].val,
					counts[0].val,
					1000000ULL,quiet);

		if ((error>10.0) || ( error<-10.0)) {
			if (!quiet) printf("Error out of range!\n");
			test_fail(test_string);
		}

		error=display_error(counts2[0].val,
					counts2[0].val,
					counts2[0].val,
					2000000ULL,quiet);

		if ((error>10.0) || ( error<-10.0)) {
			if (!quiet) printf("Error out of range!\n");
			test_fail(test_string);
		}

		if (!quiet) {
			printf("Should not have worked\n");
		}
		test_fail(test_string);
	}


	perf_evlist__for_each_evsel(evlist, evsel) {
		perf_evsel__munmap(evsel);
	}
	perf_evlist__close(evlist);
	perf_evlist__delete(evlist);
	perf_thread_map__put(threads);

	perf_evlist__for_each_evsel(evlist2, evsel) {
		perf_evsel__munmap(evsel);
	}
	perf_evlist__close(evlist2);
	perf_evlist__delete(evlist2);
	perf_thread_map__put(threads2);


	test_pass(test_string);

	return 0;
}
