/* This file attempts to see if rdpmc works on attach   */

/* by Vince Weaver, vincent.weaver _at_ maine.edu       */


char test_string[]="Testing if rdpmc attach works...";
int quiet=0;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "perf_event.h"
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

	/* Wait to be triggered */
	if (ptrace(PTRACE_TRACEME, 0, 0, 0) == 0) {
	}

	for(i=0;i<num;i++) {
		result=instructions_million();
	}

	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");


	/* Wait again */
	raise(SIGSTOP );

	return 0;
}


int main(int argc, char **argv) {

	int i;
	long page_size=getpagesize();
	double error;

	long long start_before,stop_after;

	void *addr[MAX_EVENTS];

	struct perf_event_attr pe;

	int fd[MAX_EVENTS],ret1,ret2;
	int status;

	unsigned long long values[MAX_EVENTS],enabled[MAX_EVENTS],running[MAX_EVENTS];
	unsigned long long values2[MAX_EVENTS];

	int count=2;

	pid_t pid;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if rdpmc attach works.\n\n");
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
		exit(wait_for_attach_and_loop(100));
	}

	if ( ptrace( PTRACE_ATTACH, pid, NULL, NULL ) == -1 ) {
		fprintf(stderr,"Error attaching to %d\n",pid);
		test_fail(test_string);
	}
	if ( waitpid( pid, &status, 0 ) == -1 ) {
		fprintf(stderr,"Error waitpid %d\n",pid);
		test_fail(test_string);
	}
	if ( WIFSTOPPED( status ) == 0 ) {
		fprintf(stderr,"WIFSTOPPED didn't happen %d\n",pid);
		test_fail(test_string);
	}



	/*****************************/
	/* TEST START/WORK/READ/STOP */
	/*****************************/

	/* Open event, pid1  */
	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);

	fd[0]=-1;

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

		fd[i]=perf_event_open(&pe,pid,-1,fd[0],0);
		if (fd[i]<0) {
			fprintf(stderr,"Error opening event %d\n",i);
			test_fail(test_string);
		}

		/* mmap() event */
		addr[i]=mmap(NULL,page_size, PROT_READ, MAP_SHARED,fd[i],0);
		if (addr[i] == MAP_FAILED) {
			fprintf(stderr,"Error mmap()ing event %d!\n",i);
			test_fail(test_string);
		}
	}


	/* start */
	start_before=rdtsc();

	ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);


	/* start up pid */
	if ( ptrace( PTRACE_CONT, pid, NULL, NULL ) == -1 ) {
		fprintf(stderr,"Couldn't continue %d\n",pid);
		test_fail(test_string);
	}
	if ( waitpid( pid, &status, 0 ) == -1 ) {
		fprintf(stderr,"Couldn't waitpid() %d\n",pid );
		test_fail(test_string);
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

	/* read */
	for(i=0;i<count;i++) {
		values[i] = mmap_read_self(addr[i], &enabled[i], &running[i]);
		read(fd[i],&values2[i],sizeof(long long));
	}

	/* stop */
	ret2=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	stop_after=rdtsc();

	if (ret1<0) {
		fprintf(stderr,"Error starting!\n");
		test_fail(test_string);
	}

	if (ret2<0) {
		fprintf(stderr,"Error stopping!\n");
		test_fail(test_string);
	}

	if (values[0]<0) {
		if (!quiet) printf("rdpmc support not available.\n");
		test_yellow_no(test_string);
	}

	for(i=0;i<count;i++) {
		close(fd[i]);
		munmap(addr[i],page_size);
	}

	if (!quiet) {
		printf("Trying attach: %lld cycles\n",
			stop_after-start_before);
		for(i=0;i<count;i++) {
			printf("\t* RDPMC Event %x -- count: %lld enabled %lld running: %lld\n",
				i,values[i],enabled[i],running[i]);
			printf("\t! READ Event %x -- count: %lld\n",
				i,values2[i]);
		}
	}

	if (!quiet) printf("\n");

	error=display_error(values[0],
				values[0],
				values[0],
				100000000ULL,quiet);

	if ((error>10.0) || ( error<-10.0)) {
		if (!quiet) printf("Error out of range!\n");
		test_fail(test_string);
	}

	for(i=0;i<count;i++) {
		close(fd[i]);
		munmap(addr[i],page_size);
	}

	test_pass(test_string);

	return 0;
}
