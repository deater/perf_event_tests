/* read_group_attached.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* This was fixed in 2.6.34 by                       */
/*   commit 050735b08ca8a016bbace4445fa025b88fee770b */
/*   perf: Fix exit() vs PERF_FORMAT_GROUP           */


#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include <signal.h>

#include <sys/mman.h>

#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "matrix_multiply.h"

int main(int argc, char** argv) {

	int ret,fd1,fd2,quiet,i;
	int result,status;
	pid_t pid;

	struct perf_event_attr pe;

	char test_string[]="Testing if FORMAT_GROUP works on attached processes...";

	quiet=test_quiet();

	/* fork off a child */
	pid = fork(  );

	if ( pid < 0 ) {
		fprintf(stderr,"Failed fork\n");
		test_fail(test_string);
	}

	/* our child */
	/* Set up ptrace */
	/* Then send SIGTRAP signal as a debugger would? */
	if ( pid == 0 ) {
		if (ptrace(PTRACE_TRACEME, 0, 0, 0) == 0) {
			kill(getpid(),SIGTRAP);
			naive_matrix_multiply(1);
		}
		else {
			fprintf(stderr,"Failed ptrace...\n");
		}
		return 1;
	}


	if (!quiet) {
		printf("Before 2.6.34 you could not read all sibling counts\n");
		printf("  from the group leader by specifying FORMAT_GROUP\n");
	}

	pid_t  child = wait( &status );

	/* Make sure child is stopped and waiting? */

	if (!quiet) printf( "Monitoring pid %d status %d\n",pid,child );

	if (WIFSTOPPED( status )) {
		if (!quiet) {
			printf( "Child has stopped due to signal %d (%s)\n",
				WSTOPSIG( status ), strsignal(WSTOPSIG( status )) );
		}
	}

	if (WIFSIGNALED( status )) {
		if (!quiet) {
			printf( "Child %d received signal %d (%s)\n",
				child, WTERMSIG(status),
				strsignal(WTERMSIG( status )) );
		}
	}

	/*************************************/
	/* Set up events to measure on child */
	/*************************************/

	/* set up group leader */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
	pe.disabled=1;
	pe.pinned=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,pid,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	/* setup event 2 */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_CPU_CYCLES;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe,quiet);

	fd2=perf_event_open(&pe,pid,-1,fd1,0);
	if (fd2<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd2, PERF_EVENT_IOC_RESET, 0);

	/* enable counting on child */
	ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);


	/* Restart child process */
	if (!quiet) printf("Continuing child\n");

	if ( ptrace( PTRACE_CONT, pid, NULL, NULL ) == -1 ) {
		fprintf(stderr,"Error continuing\n");
		test_fail(test_string);
	}


	/* Wait for child to exit */
	do {
		child = wait( &status );
		if (!quiet) printf( "Debugger exited wait() with %d\n", child);
		if (WIFSTOPPED( status )) {
			if (!quiet) printf( "Child has stopped due to signal %d (%s)\n",
				WSTOPSIG( status ),
				strsignal(WSTOPSIG( status )) );
		}

		if (WIFSIGNALED( status )) {
			if (!quiet) printf( "Child %d received signal %d (%s)\n",
				child, WTERMSIG(status) ,
				strsignal(WTERMSIG( status )) );
		}
	} while (!WIFEXITED( status ));


	/* disable counting on child */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	#define BUFFER_SIZE 32
	long long buffer[BUFFER_SIZE];
	for(i=0;i<BUFFER_SIZE;i++) {
		buffer[i]=-1;
	}

	result=read(fd1,buffer,BUFFER_SIZE*sizeof(long long));
	if (result<0) {
		if (!quiet) {
			fprintf(stderr,"Unexpected read result %d\n",result);
		}
		test_fail(test_string);
	}

	/* should be 1 + 2*num_events */
	/* which is 5 in our case     */
	if (result!=(5)*sizeof(long long)) {
		if (!quiet) {
			fprintf(stderr,"Unexpected read result %d (should be %zd)\n",
					result,5*sizeof(long long));
		}
		test_fail(test_string);
	}

	if (!quiet) {
		printf("Number of events: %lld\n",buffer[0]);
		for(i=0;i<buffer[0];i++) {
			printf("Value    [%d] : %lld\n",i,buffer[1+(i*2)]);
			printf("Format ID[%d] : %lld\n",i,buffer[1+((i*2)+1)]);
		}
	}

	test_pass(test_string);

	return 0;
}

