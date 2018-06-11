/* prctl_parent.c				*/
/* Test the perf-related prctl() commands	*/
/*  particularly, if prctl() starts/stops events in attached process */

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

static char test_string[]="Testing if prctl() affects remote attached events...";

static volatile int done=0;

static void our_handler(int signum,siginfo_t *info, void *uc) {

	done=1;

}

static int wait_for_attach_and_loop( int quiet ) {

	int result=0,read_result=0;
	int fd[2];
	struct perf_event_attr pe;
	long long insn_data[4];
	struct sigaction sa;

	if (!quiet) {
		printf("CHILD: setting up signal handler\n");
	}

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGUSR1, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}

	if (!quiet) {
		printf("CHILD: setting up event\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.read_format=PERF_FORMAT_ID;

	arch_adjust_domain(&pe, quiet);

	fd[0]=perf_event_open(&pe,0,-1,-1,0);
	if (fd[0]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

//	sleep(2);

//	if (!quiet) {
//		printf("CHILD: going to sleep\n");
//		fflush(stdout);
//	}

//	kill( getpid(), SIGSTOP );


	while(1) {
		result=instructions_million();
		if (done) break;
		sleep(1);
	}

	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

	if (!quiet) {
		printf("CHILD: continuing due to signal\n");
	}

	/* Read the counter */
	/* Should be 0 unless somehow parent prctl turned us on */

	read_result=read(fd[0],insn_data,2*sizeof(long long));

	if (read_result!=2*sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
	}

	if (!quiet) {
		printf("\tCHILD Read (should be 0): %lld\n",insn_data[0]);
	}

	if (!quiet) {
		printf("CHILD: exiting\n");
	}

	if (insn_data!=0) return -1;

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

	/* attach to child and make sure it is waiting */
//	if ( ptrace( PTRACE_ATTACH, pid, NULL, NULL ) == -1 ) {
//		fprintf(stderr,"Error attaching to %d\n",pid);
//		return -1;
//	}
//	if ( waitpid( pid, &status, 0 ) == -1 ) {
//		fprintf(stderr,"Error waitpid %d\n",pid);
//		return -1;
//	}
//	if ( WIFSTOPPED( status ) == 0 ) {
//		fprintf(stderr,"WIFSTOPPED didn't happen %d\n",pid);
//		test_fail(test_string);
//	}

//	if (!quiet) {
//		printf("+ Child has been stopped\n");
//		sleep(5);
//	}


	/* Setup a local event */

	if (!quiet) {
		printf("+ Setting up local event\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.read_format=PERF_FORMAT_ID;

	arch_adjust_domain(&pe, quiet);

	fd[0]=perf_event_open(&pe,0,-1,-1,0);
	if (fd[0]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	if (!quiet) {
		printf("+ Continuing child\n");
	}

        /* start up child */
//	if ( ptrace( PTRACE_CONT, pid, NULL, NULL ) == -1 ) {
//		fprintf(stderr,"Couldn't continue %d\n",pid);
//		test_fail(test_string);
//	}
//	if ( waitpid( pid, &status, 0 ) == -1 ) {
//		fprintf(stderr,"Couldn't waitpid() %d\n",pid );
  //              test_fail(test_string);
//	}
//	if ( WIFSTOPPED( status ) == 0 ) {
//		test_fail(test_string);
//	}
//	if ( WSTOPSIG( status ) != SIGSTOP ) {
//		test_fail(test_string);
//	}

	if (!quiet) {
		printf("+ Starting counting with prctl in parent\n");
	}

	prctl(PR_TASK_PERF_EVENTS_ENABLE);

	instructions_million();
	sleep(1);		// should we yield so other gets to run?


	prctl(PR_TASK_PERF_EVENTS_DISABLE);

	if (!quiet) {
		printf("+ Stopping counting with prctl in parent\n");
	}

	if (!quiet) {
		printf("+ Stopping child\n");
	}
	kill( pid, SIGUSR1 );

	read_result=read(fd[0],insn_data,2*sizeof(long long));

	if (read_result!=2*sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\t+ Read (should be 1 million): %lld\n",insn_data[0]);
	}
	old[0]=insn_data[0];

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

	/* Wait for child to finish */
	if ( waitpid( pid, &status, 0 ) == -1 ) {
		fprintf(stderr,"Couldn't waitpid() %d\n",pid );
		test_fail(test_string);
	}
	if ( WIFSTOPPED( status ) != 0 ) {
		fprintf(stderr,"Failed due to child failing!\n");
		test_fail(test_string);
	}

	test_pass(test_string);


	return 0;
}
