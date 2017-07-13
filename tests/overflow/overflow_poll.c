/* overflow_poll.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* Test using poll to catch overflow */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include <signal.h>

#include <sys/ptrace.h>

#include <sys/mman.h>

#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/wait.h>
#include <sys/poll.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#define MMAP_PAGES 8

static struct signal_counts {
	int in,out,msg,err,pri,hup,unknown,total;
} count = {0,0,0,0,0,0,0,0};

static int fd1;

int main(int argc, char** argv) {

	int ret,quiet,status;
	pid_t pid,child;

	struct perf_event_attr pe;

	void *our_mmap;
	char test_string[]="Testing catching overflow with poll()...";

	quiet=test_quiet();

	if (!quiet) printf("This tests using poll() to catch overflow.\n");

	/* fork off a child */
	pid=fork();

	if (pid<0) {
		fprintf(stderr,"Failed fork\n");
		test_fail(test_string);
	}

	/* Our child.  Set up ptrace to stop it     */
	/* originally tried kill(getpid(),SIGSTOP); */
	/* but that is unreliable?                  */
	if (pid==0) {
		if (ptrace(PTRACE_TRACEME, 0, 0, 0) == 0) {
			int c;
			kill(getpid(),SIGTRAP);

                        for(c=0;c<10;c++) {
				instructions_million();
			}
                }
                else {
                        fprintf(stderr,"Failed ptrace...\n");
                }
                return 1;
        }

	child=wait(&status);

	/* Make sure child is stopped and waiting */
	if (!quiet) printf( "Monitoring pid %d status %d\n",pid,status);

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


	/* Create a sampled event and attach to child */

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;

	/* 1 million.  Tried 100k but that was too short on */
	/* faster machines, likely triggered overflow while */
	/* poll still was being handled?                    */
	pe.sample_period=1000000;
	pe.sample_type=PERF_SAMPLE_IP;
	pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
	pe.disabled=1;
	pe.pinned=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.wakeup_events=1;

	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,child,-1,-1,0);
	if (fd1<0) {
		if (!quiet) fprintf(stderr,"Error opening leader %llx\n",pe.config);
		test_fail(test_string);
	}

	our_mmap=mmap(NULL, (1+MMAP_PAGES)*getpagesize(),
			PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);


	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);
	ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);

	if (ret<0) {
		if (!quiet) {
			fprintf(stderr,"Error with PERF_EVENT_IOC_ENABLE of group leader: "
					"%d %s\n",errno,strerror(errno));
			test_fail(test_string);
		}
	}

	struct pollfd fds[1];
	int result;

	fds[0].fd=fd1;
	fds[0].events=POLLIN|POLLHUP|POLLNVAL|POLLERR;

	/* Restart child process */
	if (!quiet) printf("Continuing child\n");

	if ( ptrace( PTRACE_CONT, pid, NULL, NULL ) == -1 ) {
		fprintf(stderr,"Error continuing\n");
		test_fail(test_string);
	}

//	kill(child,SIGCONT);

	while(1) {
		result=poll(fds,1,100);
		if (result==0) {
			waitpid(child,&status,WNOHANG);
			if (WIFEXITED(status)) break;
			if (WIFSIGNALED(status)) {
				printf("Signalled %d!\n",WTERMSIG(status));
				break;
			}
		}
		else if (result==-1) {
			printf("Error: %s\n",strerror(errno));
			break;

		}

		if (fds[0].revents&POLLIN) count.in++;
		if (fds[0].revents&POLLHUP) count.hup++;

		if (fds[0].revents&POLLERR) {
			if (!quiet) printf("Returned error!\n");
			break;
		}

		/* On 3.18 and newer we get infinite POLLHUP	*/
		/* When the child exits	rather than an error	*/

		if (fds[0].revents&POLLHUP) {
			if (!quiet) printf("Returned HUP!\n");
			break;
		}

//		printf("%d %d\n",result,fds[0].revents);
	}

	close(fd1);

	count.total=count.in+count.hup;

	if (!quiet) {
		printf("Counts, using mmap buffer %p\n",our_mmap);
		printf("\tPOLL_IN : %d\n",count.in);
		printf("\tPOLL_OUT: %d\n",count.out);
		printf("\tPOLL_MSG: %d\n",count.msg);
		printf("\tPOLL_ERR: %d\n",count.err);
		printf("\tPOLL_PRI: %d\n",count.pri);
		printf("\tPOLL_HUP: %d\n",count.hup);
		printf("\tUNKNOWN : %d\n",count.unknown);
	}

	/* I think it is exected that we get POLL_IN for each time the */
	/* wakeup value is triggered (indicating data is ready) and we */
	/* only get POLL_HUP if the monitored process exits (hangs up) */
	/* I think older (pre-3.18?) did this differently.             */

	if (count.total==0) {
		if (!quiet) printf("No overflow events generated.\n");
		test_fail(test_string);
	}

	if (count.in!=10) {
		if (!quiet) printf("Unexpected POLL_IN interrupt.\n");
		test_fail(test_string);
	}

	if (count.hup!=1) {
		if (!quiet) {
			printf("POLL_HUP value %d, expected %d.\n",
					count.hup,1);
			printf("Expected if kernel older than 3.18, "
				"as poll() would get an error rather than "
				"POLL_HUP if the monitored process detached\n");
		}
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}

