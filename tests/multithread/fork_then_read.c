/* fork_then_read.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */


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

#include "instructions_testcode.h"


int main(int argc, char** argv) {

	int ret,fd1,quiet,i;
	int result;
	pid_t pid;

	struct perf_event_attr pe;

	char test_string[]="Testing reads in forked children...";

	quiet=test_quiet();

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

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	if (!quiet) {
		printf("Testing fork behavior\n");
		printf("Even though the child runs longer, the value\n");
		printf("it reads should be that of the parent.\n");
	}

	/* enable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);

	/* Run a million */
	result=instructions_million();
	if (result==CODE_UNIMPLEMENTED) {
		test_skip(test_string);
	}

	/* fork off a child */
	pid = fork();

	if ( pid < 0 ) {
		fprintf(stderr,"Failed fork\n");
		test_fail(test_string);
	}

	/* our child */
	if ( pid == 0 ) {
		if (!quiet) printf("In child, running an extra million\n");
		/* extra million */
		result=instructions_million();
	}
	else {
		if (!quiet) printf("In parent\n");

		/* disable counting */
		ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
		if (ret<0) {
			if (!quiet) printf("Error disabling\n");
		}
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
	/* which is 3 in our case     */
	if (result!=(3)*sizeof(long long)) {
		if (!quiet) {
			fprintf(stderr,"Unexpected read result %d (should be %zd)\n",
					result,3*sizeof(long long));
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

	double error;
	long long average,high,low;
	int failure=0;

	average=high=low=buffer[1];

	error=display_error(average,high,low,1000000ULL,quiet);

	if ((error > 1.0) || (error<-1.0)) {
		failure++;
	}

	/* child */
	if (pid==0) {
		return failure;
	}
	else {
		int status;

		waitpid(pid,&status,0);

		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status)!=0) failure++;
		}
	}

	if (failure) {
		test_fail(test_string);
	}


	test_pass(test_string);

	return 0;
}
