/* signal_after_exec.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu                         */

/* Compile with gcc -O2 -Wall -o signal_after_exec signal_after_exec.c     */

/* On 2.6.39 and earlier the execd process gets no signals                 */
/* On 3.0 and normal it does, which quickly kills the program with SIGIO   */

/* Commit that caused the problem f506b3dc0ec454a16d40cab9ee5d75435b39dc50 */
/* Both before and after behavior are apparently unintentional             */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <signal.h>

#include <sys/mman.h>

#include <sys/ioctl.h>

#include <sys/wait.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"


static int count=0;

static void our_handler(int signum,siginfo_t *oh, void *blah) {

	int ret,fd1;

	fd1=oh->si_fd;

	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);

	count++;

	ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH,1);

	(void)ret;
}

double busywork(int count) {

	int i;
	double sum=0.0012;

	for(i=0;i<count;i++) {
		sum+=0.01;
	}
	return sum;

}


int main(int argc, char** argv) {

	int fd,ret,quiet;
	double result;

	struct perf_event_attr pe;
	struct sigaction sa;
	pid_t child;

	char test_string[]="Testing if overflow signals survive exec...";

	quiet=test_quiet();

	if (argc>1) {
		result=busywork(10000000);
		if (!quiet) printf("Count after exec=%d (%lf)\n",count,result);
		exit(0);
	}

	child=fork();

	if (child!=0) {
		int status;

		wait(&status);

		if (!quiet) {
			printf("Status: %d\n\n",status);
		}

		if (status==0) {
			if (!quiet) printf("Old behavior\n");
			test_yellow_old_behavior(test_string);
		}
		else {
			if (!quiet) printf("New behavior\n");
			test_pass(test_string);
		}
		exit(0);
	}

	if (!quiet) {
		printf("\nOn 2.6.39 and earlier the exec'd process gets no signals.\n");
		printf("On 3.0 and later the exec'd process gets a signal, which\n");
		printf("  is not handled and it dies with SIGIO.\n\n");
	}

	/* set up signal handler */
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		test_fail(test_string);
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.sample_period=100000;
	pe.sample_type=PERF_SAMPLE_IP;
	pe.read_format=0;
	pe.disabled=1;
	pe.pinned=1;

	pe.wakeup_events=1;

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		fprintf(stderr,"Error opening\n");
		test_fail(test_string);
		exit(1);
	}

	/* setup event 2 to have overflow signals */
	fcntl(fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd, F_SETSIG, SIGIO);
	fcntl(fd, F_SETOWN,getpid());

	ioctl(fd, PERF_EVENT_IOC_RESET, 0);

	/* enable counting */
	ret=ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

	result=busywork(10000000);

	if (!quiet) {
		printf("Count before exec=%d (%lf)\n",count,result);
	}

	/* exec ourselves, but call a busy function */
	execl(argv[0],argv[0],"busy",NULL);

	(void) ret;

	return 0;
}
