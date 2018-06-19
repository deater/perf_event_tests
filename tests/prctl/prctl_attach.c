/* prctl_parent.c				*/
/* Test the perf-related prctl() commands	*/
/*  particularly, if prctl() starts/stops attached events */

/* by Vince Weaver   vincent.weaver _at_ maine.edu  */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

#include <errno.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"
#include "instructions_testcode.h"

static char test_string[]="Testing if prctl() affects attached events...";

static int wait_for_attach_and_loop( int quiet ) {

	int result=0,i;

	if (!quiet) {
		printf("CHILD: stopping\n");
	}
	/* Wait to be triggered */
	if (ptrace(PTRACE_TRACEME, 0, 0, 0) == 0) {
	}

	if (!quiet) {
		printf("CHILD: continuing\n");
	}

	for(i=0;i<5;i++) {
		result=instructions_million();
	}
	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

	if (!quiet) {
		printf("CHILD: done and exiting\n");
	}

	return 0;
}


int main(int argc, char** argv) {

	int fd[2];
	int quiet;
	int read_result,status;
	long long insn_data[4],old[4];
	pid_t pid;

	struct perf_event_attr pe;

	quiet=test_quiet();

	if (!quiet) {
		printf("This tests if prctl() affects remote attached events.\n\n");
	}

	/********************************************************/
	/* Test 1: Two processes, One Event			*/
	/********************************************************/

	if (!quiet) {
		printf("Two processes, One event\n");
	}

	/*********************************/
	/* start and pause a child       */
	/*********************************/

	if (!quiet) {
		printf("+ Forking child\n");
	}

	/* fork a child */
	pid = fork();
	if ( pid < 0 ) {
		fprintf(stderr,"Failed fork\n");
		test_fail(test_string);
	}
	else if (pid==0) {
		/* in child */
		wait_for_attach_and_loop(quiet);
		return 0;
	}

	/* Wait for child to stop */
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

	/* Setup an attached event */

	if (!quiet) {
		printf("+ Setting up attached event\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.read_format=PERF_FORMAT_ID;

	arch_adjust_domain(&pe, quiet);

	fd[0]=perf_event_open(&pe,pid,-1,-1,0);
	if (fd[0]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}


	if (!quiet) {
		printf("+ Enabling all events with prctl()\n");
	}

	prctl(PR_TASK_PERF_EVENTS_ENABLE);


	if (!quiet) {
		printf("+ Continuing child\n");
	}

	/* start up pid */
	if ( ptrace( PTRACE_CONT, pid, NULL, NULL ) == -1 ) {
		fprintf(stderr,"Couldn't continue %d\n",pid);
		test_fail(test_string);
	}

	/* Wait for it to finish */
	if ( waitpid( pid, &status, 0 ) == -1 ) {
		fprintf(stderr,"Couldn't waitpid() %d %s\n",
			pid,strerror(errno) );
		test_fail(test_string);
	}

	if (!quiet) {
		printf("+ Stopping counting with prctl in parent\n");
	}

	prctl(PR_TASK_PERF_EVENTS_DISABLE);

	read_result=read(fd[0],insn_data,2*sizeof(long long));

	if (read_result!=2*sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\t+ Read (should be around 5 million): %lld\n",insn_data[0]);
	}
	old[0]=insn_data[0];

	if (insn_data[0]<10000) {
		if (!quiet) {
			printf("Should have been much larger!\n");
			test_fail(test_string);
		}
	}

	if (insn_data[0]>10000000) {
		if (!quiet) {
			printf("Should have been much smaller!\n");
			test_fail(test_string);
		}
	}


	instructions_million();

	read_result=read(fd[0],insn_data,2*sizeof(long long));

	if (read_result!=2*sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tReading again after disable\n");
		printf("\t+ Read Instructions (should match last): %lld (%lld)\n",
			insn_data[0],old[0]);
	}

	if (old[0]==0) {
		test_fail(test_string);
	}

	if (old[0]!=insn_data[0]) {
		test_fail(test_string);
	}

	close(fd[0]);

	test_pass(test_string);


	return 0;
}
