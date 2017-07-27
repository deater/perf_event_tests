/* This file attempts to see if rdpmc works on attach   */
/* This is a bug turned up by the PAPI multiattach.c test */
/* We've minimized it as much as possible */

/* The failing pattern seems to be OPEN_ATTACH/ENABLE/DISABLE/ENABLE/RDPMC */

/* by Vince Weaver, vincent.weaver _at_ maine.edu       */

/* This bug still exists as of 4.13-rc2 (though it has been reported) */
/* The problem is if you attach to a process in PERF_EVENT_STATE_INACTIVE */
/* and then enable/disable the event (while the process is not running) */
/* the event->tstamp_enabled is updated but event->tstamp_stopped */
/* is not. These are subtracted, and then enabled field goes negative.*/


static char test_string[]="Testing if minimized rdpmc papi-multi-attach works...";
static int quiet=0;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "perf_event.h"
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

	long page_size=getpagesize();
	long long start_before,stop_after;
	void *addr;
	struct perf_event_attr pe;
	int fd,ret1,ret2;
	int status;
	int i;

	unsigned long long values,enabled,running;

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

	fd=perf_event_open(&pe,pid,-1,-1,0);
	if (fd<0) {
		fprintf(stderr,"Error opening event\n");
		test_fail(test_string);
	}

	/* mmap() event */
	addr=mmap(NULL,page_size, PROT_READ, MAP_SHARED,fd,0);
	if (addr == MAP_FAILED) {
		fprintf(stderr,"Error mmap()ing event\n");
		test_fail(test_string);
	}

	if (!quiet) printf("+ We've attached one event to %d\n",pid);


	/* Magic */
	for(i=0;i<2;i++) {
		ret1=ioctl(fd, PERF_EVENT_IOC_ENABLE,0);
		ret1=ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
	}

	/* start */
	start_before=rdtsc();

	ret1=ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

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
	values = mmap_read_self(addr, &enabled, &running);


	/* stop */
	ret2=ioctl(fd, PERF_EVENT_IOC_DISABLE,0);

	stop_after=rdtsc();

	if (ret1<0) {
		fprintf(stderr,"Error starting!\n");
		test_fail(test_string);
	}

	if (ret2<0) {
		fprintf(stderr,"Error stopping!\n");
		test_fail(test_string);
	}

	if (values<0) {
		if (!quiet) printf("rdpmc support not available.\n");
		test_yellow_no(test_string);
	}


	close(fd);
	munmap(addr,page_size);

	if (!quiet) {
		printf("Trying attach: %lld cycles\n",
			stop_after-start_before);
		printf("\t* RDPMC 1 Event %x -- count: %lld enabled %llx running: %llx\n",
				i,values,enabled,running);
	}

	if (enabled!=running) {
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
