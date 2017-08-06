/* record_mmap_data.c  */
/* by Vince Weaver   <vincent.weaver@maine.edu> */

/* An attempt to figure out the PERF_RECORD_MMAP_DATA code */

/* When modified with .mmap_data			*/
/* This returns for all non-executable mmaps		*/

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

#define SAMPLE_FREQUENCY 1000000000

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


static int generate_mmaps(int quiet, int perf_fd, int *expected) {

	char *mmap1,*mmap2,*mmap3,*mmap4,*mmap5,*mmap6,*mmap7;
	size_t pagesize = sysconf(_SC_PAGESIZE);
	int mmap_fails=0;
	int mmap_pages=1+MMAP_DATA_SIZE;
	int fd2;

	*expected=0;

	/* Anonymous read/write, should count */
	if (!quiet) printf("\t+ anon read/write, should be counted.\n");
	mmap1=mmap(NULL, pagesize,
			PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
	if (mmap1==MAP_FAILED) {
		if (!quiet) {
			printf("\t\tError! mmap1() failed %s!\n",strerror(errno));
		}
		mmap_fails++;
	}
	else {
		if (!quiet) printf("\t\t%p\n",mmap1);
		(*expected)++;
	}

	/* perf_event read/write, should count */
	if (!quiet) printf("\t+ perf read/write, should be counted.\n");

	mmap2=mmap(NULL, mmap_pages*pagesize,
			PROT_READ|PROT_WRITE, MAP_SHARED, perf_fd, 0);
	if (mmap2==MAP_FAILED) {
		if (!quiet) {
			printf("\t\tError! mmap2() failed %s!\n",strerror(errno));
		}
		mmap_fails++;
	} else {
		if (!quiet) printf("\t\t%p\n",mmap2);
		(*expected)++;
	}

	/* perf_event read/write, should count */
	if (!quiet) printf("\t+ zero read/write, should be counted.\n");

	fd2=open("/dev/zero",O_RDWR);
	mmap3=mmap(NULL, mmap_pages*pagesize,
			PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);
	if (mmap3==MAP_FAILED) {
		if (!quiet) {
			printf("\t\tError! mmap3() failed %s!\n",strerror(errno));
		}
		mmap_fails++;
	} else {
		if (!quiet) printf("\t\t%p\n",mmap3);
		(*expected)++;
	}

	/* Anonymous read/write/exec, *should not*  count */
	if (!quiet) printf("\t+ anon read/write/exec, *should not* be counted.\n");
	mmap4=mmap(NULL, pagesize,
			PROT_EXEC|PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
	if (mmap4==MAP_FAILED) {
		if (!quiet) {
			printf("\t\tError! mmap4() failed %s!\n",strerror(errno));
		}
		mmap_fails++;
	} else {
		if (!quiet) printf("\t\t%p\n",mmap4);
	}

	/* perf_event read/write/exec, *should not* count */
	if (!quiet) printf("\t+ perf read/write/exec, *should not* be counted.\n");
	mmap5=mmap(NULL, mmap_pages*pagesize,
		PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, perf_fd, 0);
	if (mmap5==MAP_FAILED) {
		if (!quiet) {
			printf("\t\tError! mmap5() failed %s!\n",strerror(errno));
		}
		mmap_fails++;
	} else {
		if (!quiet) printf("\t\t%p\n",mmap5);
	}

	/* zero read/write/exec, *should not* count */
	if (!quiet) printf("\t+ zero read/write/exec, *should not* be counted.\n");
	mmap6=mmap(NULL, mmap_pages*pagesize,
		PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);
	if (mmap6==MAP_FAILED) {
		if (!quiet) {
			printf("\t\tError! mmap6() failed %s!\n",strerror(errno));
		}
		mmap_fails++;
	}
	else {
		if (!quiet) printf("\t\t%p\n",mmap6);
	}

	/* Anonymous read/write/exec, *should not* count */
	if (!quiet) printf("\t+ anon read/write/exec, *should not* be counted.\n");
	mmap7=mmap(NULL, pagesize*2,
			PROT_EXEC|PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
	if (mmap7==MAP_FAILED) {
		if (!quiet) {
			printf("\t\tError! mmap7() failed %s!\n",strerror(errno));
		}
		mmap_fails++;
	}
	else {
		if (!quiet) printf("\t\t%p\n",mmap7);
	}

	if (!quiet) printf("\n");

	return mmap_fails;
}



int main(int argc, char **argv) {

	int ret;
	int fd;
	int mmap_pages=1+MMAP_DATA_SIZE;
	int events_read;
	int version;
	int mmap_fails=0;

	struct perf_event_attr pe;

	struct sigaction sa;
	int expected=0;
	char test_string[]="Testing PERF_RECORD_MMAP with mmap_data...";

	quiet=test_quiet();

	if (!quiet) {
		printf("This tests PERF_RECORD_MMAP_DATA samples:\n");
		printf("\twe set up such an event and then run mmap() with\n");
		printf("\tvarious combinations of paramaters\n\n");
	}

        version=get_kernel_version();

        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_sigaction = our_handler;
        sa.sa_flags = SA_SIGINFO;

        if (sigaction( SIGIO, &sa, NULL) < 0) {
                if (!quiet) printf("Error setting up signal handler\n");
                exit(1);
        }

        /* Set up Instruction Event */

        memset(&pe,0,sizeof(struct perf_event_attr));

        /* PERF_COUNT_SW_DUMMY not available until 3.12 */
        if (version<0x30c00) {
                pe.type=PERF_TYPE_HARDWARE;
                pe.config=PERF_COUNT_HW_INSTRUCTIONS;
        }
        else {
                pe.type=PERF_TYPE_SOFTWARE;
                pe.config=PERF_COUNT_SW_DUMMY;
	}

        pe.size=sizeof(struct perf_event_attr);
        pe.sample_period=SAMPLE_FREQUENCY;

        pe.read_format=0;
        pe.disabled=1;
        pe.pinned=1;
        pe.exclude_kernel=1;
        pe.exclude_hv=1;
        pe.wakeup_events=1;

	pe.mmap_data=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		if (!quiet) {
			printf("Problem opening leader %s\n",
				strerror(errno));
		}
		/* Introduced in 2.6.36 */
		if (version<0x20624) {
			if (!quiet) {
				printf("mmap_data support not added "
					"until Linux 2.6.36\n");
			}

			test_skip(test_string);
		}

		test_fail(test_string);
	}

	our_mmap=mmap(NULL, mmap_pages*getpagesize(),
		PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (our_mmap==MAP_FAILED) {
		if (!quiet) {
			printf("mmap() failed %s!\n",strerror(errno));
		}
		test_fail(test_string);
	} else {
		if (!quiet) {
			printf("\tperf_event mmap is %p\n",our_mmap);
		}
	}


	fcntl(fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd, F_SETSIG, SIGIO);
	fcntl(fd, F_SETOWN,getpid());

	ioctl(fd, PERF_EVENT_IOC_RESET, 0);

	ret=ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

	if (ret<0) {
		if (!quiet) {
			printf("Error with PERF_EVENT_IOC_ENABLE "
				"of group leader: %d %s\n",
				errno,strerror(errno));
			test_fail(test_string);
		}
	}

	/* The actual thing to measure */
	instructions_million();

	/* generate some mmap_calls() */
	mmap_fails=generate_mmaps(quiet,fd,&expected);

	instructions_million();
	/* Done measuring */



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

	if (events_read!=expected) {
		if (!quiet) printf("Wrong number of events!  Expected %d but got %d\n",
			expected,events_read);
		test_fail(test_string);
	}

	if (mmap_fails) {
		if (!quiet) printf("Some mmaps failed\n");
//		test_warn(test_string);
	}

	test_pass(test_string);

	return 0;
}
