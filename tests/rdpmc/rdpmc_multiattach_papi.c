/* This file attempts to see if rdpmc works on attach   */
/* This is a bug turned up by the PAPI multiattach.c test */
/* The failing pattern seems to be OPEN_ATTACH/ENABLE/DISABLE/ENABLE/RDPMC */

/* by Vince Weaver, vincent.weaver _at_ maine.edu       */

/* This bug was fixed by 9b231d9f47c6114d317ce28cff92a74ad80547f5	*/
/* in Linux 4.14							*/

/* The problem is if you attach to a process in PERF_EVENT_STATE_INACTIVE */
/* and then enable/disable the event (while the process is not running) */
/* the event->tstamp_enabled is updated but event->tstamp_stopped */
/* is not. These are subtracted, and then enabled field goes negative.*/


static char test_string[]="Testing if rdpmc papi-multi-attach works...";
static int quiet=0;

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

	int i;
	long page_size=getpagesize();
	double error;

	long long start_before,stop_after;

	void *addr[MAX_EVENTS],*addr2[MAX_EVENTS];

	struct perf_event_attr pe;

	int fd[MAX_EVENTS],fd2[MAX_EVENTS],ret1,ret2;
	int status;

	unsigned long long values[MAX_EVENTS],enabled[MAX_EVENTS],running[MAX_EVENTS];
	unsigned long long values2[MAX_EVENTS],enabled2[MAX_EVENTS],running2[MAX_EVENTS];

	int count=2;

	pid_t pid,pid2;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if papi-style multi-attach works.\n\n");
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


	if (!quiet) {
		printf("* We have opened two children: %d, %d\n\n",pid,pid2);
	}


	/*****************************/
	/* TEST START/WORK/READ/STOP */
	/*****************************/

	/* Open event, pid1  */
	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config1=ARCH_EVENT_CONFIG1_VAL;

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

	if (!quiet) {
		printf("+ We've attached two events to %d\n",pid);
	}

	/* Open event, pid2  */
	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config1=ARCH_EVENT_CONFIG1_VAL;

	fd2[0]=-1;

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

		fd2[i]=perf_event_open(&pe,pid2,-1,fd2[0],0);
		if (fd2[i]<0) {
			fprintf(stderr,"Error opening event %d\n",i);
			test_fail(test_string);
		}

		/* mmap() event */
		addr2[i]=mmap(NULL,page_size,
				PROT_READ, MAP_SHARED,fd2[i],0);
		if (addr2[i] == MAP_FAILED) {
			fprintf(stderr,"Error mmap()ing event %d!\n",i);
			test_fail(test_string);
		}
	}

	if (!quiet) {
		printf("+ We've attached two events to %d\n",pid2);
	}

	for(i=0;i<2;i++) {
		ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);
		ret1=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);
		ret1=ioctl(fd[0], PERF_EVENT_IOC_RESET,0);
	}

	/* start */
	start_before=rdtsc();

	/* PAPI does this */
	for(i=0;i<count;i++) {
		ret1=ioctl(fd[i], PERF_EVENT_IOC_RESET,0);
	}

	ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

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

	/* start measuring pid2 */

	/* PAPI does this */
	for(i=0;i<count;i++) {
		ret1=ioctl(fd2[i], PERF_EVENT_IOC_RESET,0);
	}

	ret1=ioctl(fd2[0], PERF_EVENT_IOC_ENABLE,0);

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
	for(i=0;i<count;i++) {
		values[i] = mmap_read_self(addr[i], &enabled[i], &running[i]);
	}

	/* stop */
	ret2=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);
	ret2=ioctl(fd2[0], PERF_EVENT_IOC_DISABLE,0);

	for(i=0;i<count;i++) {
		values2[i] = mmap_read_self(addr2[i], &enabled2[i], &running2[i]);
	}

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
		close(fd2[i]);
		munmap(addr[i],page_size);
		munmap(addr2[i],page_size);
	}

	if (!quiet) {
		printf("Trying attach: %lld cycles\n",
			stop_after-start_before);
		for(i=0;i<count;i++) {
			if (values[i]==-1) {
				printf("\t* RDPMC 1 Event %x -- rdpmc not supported\n",i);
			}
			else {
				printf("\t* RDPMC 1 Event %x -- count: %lld enabled %llx running: %llx\n",
					i,values[i],enabled[i],running[i]);
			}
		}
	}

	for(i=0;i<count;i++) {
		if ((values[i]!=-1) && (enabled[i]!=running[i])) {
			if (!quiet) printf("enabled doesn't match running!\n");
			if (check_linux_version_newer(4,14,0)) {
				test_fail(test_string);
			}
			else {
				test_known_kernel_bug(test_string);
			}
		}
	}

	if (!quiet) {
		for(i=0;i<count;i++) {
			if (values2[i]==-1) {
				printf("\t* RDPMC 2 Event %x -- rdpmc not supported\n",i);
			}
			else {
				printf("\t* RDPMC 2 Event %x -- count: %lld enabled %llx running: %llx\n",
					i,values2[i],enabled2[i],running2[i]);
			}
		}
	}

	if (!quiet) printf("\n");

	if ((values[0]!=-1) && (values2[0]!=-1)) {

		error=display_error(values[0],
					values[0],
					values[0],
					1000000ULL,quiet);

		if ((error>10.0) || ( error<-10.0)) {
			if (!quiet) printf("Error out of range!\n");
			test_fail(test_string);
		}

		error=display_error(values2[0],
					values2[0],
					values2[0],
					2000000ULL,quiet);

		if ((error>10.0) || ( error<-10.0)) {
			if (!quiet) printf("Error out of range!\n");
			test_fail(test_string);
		}

		for(i=0;i<count;i++) {
			close(fd[i]);
			munmap(addr[i],page_size);
		}
	}

	test_pass(test_string);

	return 0;
}
