/* record_comm_exec.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* An attempt to figure out the PERF_RECORD_COMM_EXEC code */

/* The COMM is the current process name. */
/* This can be changed by:		 */
/*	+ prctl(PR_SET_NAME), 		 */
/*	+ a write to /proc/self/commm	 */
/*	+ or by exec()ing a new program. */

/* The RECORD_COMM_EXEC code was added in Linux 3.16 */
/* To allow tools to diferentiate EXEC from the other cases. */


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

#include <sys/ptrace.h>
#include <sys/wait.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"
#include "parse_record.h"

#define SAMPLE_FREQUENCY 100000

#define MMAP_DATA_SIZE 8

static int count_total=0;
static char *our_mmap;
static long long prev_head;
static int quiet;
static long long global_sample_type;
static long long global_sample_regs_user;

static void our_handler(int signum, siginfo_t *info, void *uc) {

	int ret;

	int fd = info->si_fd;

	ret=ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

	prev_head=perf_mmap_read(our_mmap,MMAP_DATA_SIZE,prev_head,
		global_sample_type,0,global_sample_regs_user,
		NULL,quiet,NULL,RAW_NONE);

	count_total++;

	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH, 1);

	(void) ret;

}


int main(int argc, char **argv) {

	int ret,status;
	int fd;
	int mmap_pages=1+MMAP_DATA_SIZE;
	int events_read;
	int child;
	int version;

	struct perf_event_attr pe;

	struct sigaction sa;
	char test_string[]="Testing PERF_RECORD_COMM_EXEC...";

	quiet=test_quiet();

	if (!quiet) printf("This tests PERF_RECORD_COMM_EXEC samples:\n");

        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_sigaction = our_handler;
        sa.sa_flags = SA_SIGINFO;

        if (sigaction( SIGIO, &sa, NULL) < 0) {
                fprintf(stderr,"Error setting up signal handler\n");
                exit(1);
        }

	/* Fork child to measure */
	/* We do this in a child as we have to exec */

	child=fork();
	if (child==0) {
		FILE *fff;
		if (ptrace(PTRACE_TRACEME, 0, 0, 0) == 0) {

			kill(getpid(),SIGTRAP);

			/* The actual thing to measure */
			instructions_million();

			/* prctl */
			if (!quiet) printf("\tprctl(PR_SET_NAME,\"vmw\");\n");
			prctl(PR_SET_NAME,"vmw");

			/* /proc/self/comm */
			if (!quiet) printf("\tcat \"krg krg krg\" > /proc/self/comm\n");
			fff=fopen("/proc/self/comm","w");
			if (fff!=NULL) {
				fprintf(fff,"krg krg krg");
				fclose(fff);
			}

			/* exec */
			if (!quiet) printf("\texecl(\"/bin/false\"); [should have PERF_RECORD_MISC_COMM_EXEC set]\n");
			execl("/bin/false","/bin/true",NULL);

			instructions_million();
			/* Done measuring */
		}
                else {
                        fprintf(stderr,"Failed ptrace...\n");
                }
                return 1;
	}

	/* wait for child to stop */
	child=wait(&status);


        /* Set up Instruction Event */

        memset(&pe,0,sizeof(struct perf_event_attr));

        pe.type=PERF_TYPE_SOFTWARE;
        pe.size=sizeof(struct perf_event_attr);
        pe.config=PERF_COUNT_SW_DUMMY;
        pe.sample_period=SAMPLE_FREQUENCY;

        pe.read_format=0;
        pe.disabled=1;
        pe.pinned=1;
        pe.exclude_kernel=1;
        pe.exclude_hv=1;
        pe.wakeup_events=1;

	pe.comm_exec=1;
	pe.comm=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,child,-1,-1,0);
	if (fd<0) {
		if (!quiet) {
			fprintf(stderr,"Problem opening leader %s\n",
				strerror(errno));
		}

		version=get_kernel_version();

		/* Introduced in 3.16 */
		if (version<0x31000) {

			if (!quiet) {
				fprintf(stderr,"comm_exec support not added until Linux 3.16\n");
			}
			test_fail_kernel(test_string);
		}

		test_fail(test_string);
	}

	our_mmap=mmap(NULL, mmap_pages*getpagesize(),
		PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (our_mmap==MAP_FAILED) {
		fprintf(stderr,"mmap() failed %s!\n",strerror(errno));
		test_fail(test_string);
	}


	fcntl(fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd, F_SETSIG, SIGIO);
	fcntl(fd, F_SETOWN,getpid());

	ioctl(fd, PERF_EVENT_IOC_RESET, 0);

	ret=ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

	if (ret<0) {
		if (!quiet) {
			fprintf(stderr,"Error with PERF_EVENT_IOC_ENABLE "
				"of group leader: %d %s\n",
				errno,strerror(errno));
			exit(1);
		}
	}

	/* restart child */
	if ( ptrace( PTRACE_CONT, child, NULL, NULL ) == -1 ) {
		fprintf(stderr,"Error continuing child\n");
		test_fail(test_string);
	}

	/* Wait for child to finish */
	waitpid(child,&status,0);

	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH,0);

	if (!quiet) {
                printf("Counts %d, using mmap buffer %p\n",count_total,our_mmap);
        }

	/* Drain any remaining events */
	prev_head=perf_mmap_read(our_mmap,MMAP_DATA_SIZE,prev_head,
                global_sample_type,0,global_sample_regs_user,
                NULL,quiet,&events_read,RAW_NONE);

	munmap(our_mmap,mmap_pages*getpagesize());

	close(fd);

#define EXPECTED_EVENTS 3

	if (events_read!=EXPECTED_EVENTS) {
		if (!quiet) fprintf(stderr,"Wrong number of events!  Expected %d but got %d\n",
			EXPECTED_EVENTS,events_read);
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}
