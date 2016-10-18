/* sample_data_src.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* An attempt to figure out the PERF_SAMPLE_DATA_SRC code */

/* In general this is only set on recent Intel CPUs with PEBS (?) */
/* It also has to be an event that supports it.                   */

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

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "matrix_multiply.h"
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

	int ret;
	int fd;
	int result,precise_ip;
	int mmap_pages=1+MMAP_DATA_SIZE;
	char event_name[BUFSIZ];

	struct perf_event_attr pe;

	struct sigaction sa;
	char test_string[]="Testing PERF_SAMPLE_DATA_SRC...";

	quiet=test_quiet();

	if (!quiet) printf("This tests PERF_SAMPLE_DATA_SRC samples\n");

	memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_sigaction = our_handler;
        sa.sa_flags = SA_SIGINFO;

        if (sigaction( SIGIO, &sa, NULL) < 0) {
                fprintf(stderr,"Error setting up signal handler\n");
                exit(1);
        }

	/* Set up Appropriate Event */
	memset(&pe,0,sizeof(struct perf_event_attr));

	result=get_latency_load_event(&pe.config,&pe.config1,
				&precise_ip,event_name);
	if (result<0) {
		if (!quiet) fprintf(stderr,"No load latency event available, trying instructions (probably will return 0)\n");
		pe.type=PERF_TYPE_HARDWARE;
		pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	}
	else {
		pe.type=PERF_TYPE_RAW;
		if (!quiet) printf("Using event %s\n",event_name);
	}

	pe.size=sizeof(struct perf_event_attr);
	pe.precise_ip=precise_ip;

        pe.size=sizeof(struct perf_event_attr);
        pe.sample_period=SAMPLE_FREQUENCY;
        pe.sample_type=PERF_SAMPLE_IP | PERF_SAMPLE_WEIGHT |
			PERF_SAMPLE_DATA_SRC;

	global_sample_type=pe.sample_type;

        pe.read_format=0;
        pe.disabled=1;
        pe.pinned=1;
        pe.exclude_kernel=1;
        pe.exclude_hv=1;
        pe.wakeup_events=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		if (!quiet) {
			fprintf(stderr,"Problem opening leader %s\n",
				strerror(errno));
			test_fail(test_string);
		}
	}
	our_mmap=mmap(NULL, mmap_pages*getpagesize(),
		PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

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

	naive_matrix_multiply(quiet);

	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH,0);

	if (!quiet) {
                printf("Counts %d, using mmap buffer %p\n",count_total,our_mmap);
        }

	if (count_total==0) {
		if (!quiet) printf("No overflow events generated.\n");
		test_fail(test_string);
	}
	munmap(our_mmap,mmap_pages*getpagesize());

	close(fd);

	test_pass(test_string);

	return 0;
}
