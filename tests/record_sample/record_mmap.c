/* record_mmap.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* An attempt to figure out the PERF_RECORD_MMAP code */

/* You also need to have one of .mmap, .comm, or .task enabled */


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

#include "asm/perf_regs.h"



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
		NULL,quiet,NULL);

	count_total++;

	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH, 1);

	(void) ret;

}


int main(int argc, char **argv) {

	int ret;
	int fd;
	int mmap_pages=1+MMAP_DATA_SIZE;
	int events_read;

	struct perf_event_attr pe;

	struct sigaction sa;
	char test_string[]="Testing PERF_RECORD_MMAP...";

	quiet=test_quiet();

	if (!quiet) printf("This tests PERF_RECORD_MMAP samples:\n");

        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_sigaction = our_handler;
        sa.sa_flags = SA_SIGINFO;

        if (sigaction( SIGIO, &sa, NULL) < 0) {
                fprintf(stderr,"Error setting up signal handler\n");
                exit(1);
        }

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

	pe.mmap=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		if (!quiet) {
			fprintf(stderr,"Problem opening leader %s\n",
				strerror(errno));
		}

		test_fail(test_string);
	}

	our_mmap=mmap(NULL, mmap_pages*4096,
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

	/* The actual thing to measure */
	instructions_million();

	{

		char *mmap1,*mmap2,*mmap3,*mmap4;

		/* Anonymous read/write, should not count */
		if (!quiet) printf("\t+ anon read/write, should not count.\n");
		mmap1=mmap(NULL, 4096,
			PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
		if (mmap1==MAP_FAILED) {
			fprintf(stderr,"mmap() failed %s!\n",strerror(errno));
			test_fail(test_string);
		}

		/* perf_event read/write, should not count */
		if (!quiet) printf("\t+ perf read/write, should not count.\n");
		mmap2=mmap(NULL, mmap_pages*4096,
			PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		if (mmap2==MAP_FAILED) {
			fprintf(stderr,"mmap() failed %s!\n",strerror(errno));
			test_fail(test_string);
		}

		/* Anonymous read/write/exec, *should* count */
		if (!quiet) printf("\t+ anon read/write/exec, *should* count.\n");
		mmap3=mmap(NULL, 4096,
			PROT_EXEC|PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
		if (mmap3==MAP_FAILED) {
			fprintf(stderr,"mmap() failed %s!\n",strerror(errno));
			test_fail(test_string);
		}

		/* perf_event read/write/exec, *should* count */
		if (!quiet) printf("\t+ perf read/write/exec, *should* count.\n");
		mmap4=mmap(NULL, mmap_pages*4096,
			PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		if (mmap4==MAP_FAILED) {
			fprintf(stderr,"mmap() failed %s!\n",strerror(errno));
			test_fail(test_string);
		}

	}


	instructions_million();
	/* Done measuring */



	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH,0);

	if (!quiet) {
                printf("Counts %d, using mmap buffer %p\n",count_total,our_mmap);
        }

	/* Drain any remaining events */
	prev_head=perf_mmap_read(our_mmap,MMAP_DATA_SIZE,prev_head,
                global_sample_type,0,global_sample_regs_user,
                NULL,quiet,&events_read);

	munmap(our_mmap,mmap_pages*4096);

	close(fd);

#define EXPECTED_EVENTS 2

	if (events_read!=EXPECTED_EVENTS) {
		if (!quiet) fprintf(stderr,"Wrong number of events!  Expected %d but got %d\n",
			EXPECTED_EVENTS,events_read);
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}
