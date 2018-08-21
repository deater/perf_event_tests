/*
	Attempt to dump AMD IBS Execute (OP) records.

	Unlike Intel PEBS, the raw MSR results are dumped into a RAW_SAMPLE
	and not split out into the vaious other SAMPLE types.
*/

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


#define SAMPLE_FREQUENCY 4000

#define MMAP_DATA_SIZE 8

/* Global vars as I'm lazy */
static int count_total=0;
static char *our_mmap;
static long sample_type;
static long read_format;
static int quiet;
static long long prev_head;


static void our_handler(int signum, siginfo_t *info, void *uc) {

	int ret;

	int fd = info->si_fd;

	ret=ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

	prev_head=perf_mmap_read(
		our_mmap,
		MMAP_DATA_SIZE,
		prev_head,
		sample_type,
		read_format,
		0,	/* reg_mask */
		NULL,	/*validate */
		quiet,
		NULL,	/* events read */
		RAW_IBS_OP);	/* RAW type */

	count_total++;

	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH, 1);

	(void) ret;

}


int main(int argc, char **argv) {

	int ret;
	int fd;
	int mmap_pages=1+MMAP_DATA_SIZE;
	FILE *fff;
	struct perf_event_attr pe;

	struct sigaction sa;
	char test_string[]="Testing AMD IBS samples...";

	quiet=test_quiet();

	if (!quiet) printf("This tests AMD IBS op...\n");

	/* TODO: detect we have AMD CPU */


	/* Set up signal handler */
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}

        /* Set up Event */

        memset(&pe,0,sizeof(struct perf_event_attr));

	sample_type=PERF_SAMPLE_IP |
			PERF_SAMPLE_WEIGHT |
			PERF_SAMPLE_ADDR |
			PERF_SAMPLE_RAW;
	read_format=0;

	pe.size=sizeof(struct perf_event_attr);



	/* Use a proper event */
	/* ./perf record -a -e ibs_fetch/rand_en=1/GH /bin/ls */

	fff=fopen("/sys/devices/ibs_op/type","r");
	if (fff==NULL) {
		fprintf(stderr,"Could not open ibs_op PMU\n");
		exit(1);
	}
	fscanf(fff,"%d",&pe.type);
	fclose(fff);



	/************************************************/
	/* Set up the event				*/
	/************************************************/
	/* See MSRC001_1033 IBS Execution Control (SC_IBS_CTL) */
	/* Writable fields */
	/* 58:32 = IbsOpCurCnt */
	/* 26:20 = IbsOpMaxCnt */
	/* 19    = IbsOpCntCtrl */
	/* 18    = IbsOpVal (micro-op sample valid) */
	/* 17    = IbsOpEn  (micro-op sampling enable) */
	/* 15:0  = IbsOpMaxCnt.  Shifted left by 4 and then tack 26:20 to front */

	/* ibs_op/cnt_ctl=1/GH */
	pe.config = 0x80000;

        pe.sample_period=SAMPLE_FREQUENCY;
        pe.sample_type=sample_type;

        pe.read_format=read_format;
        pe.disabled=1;
//        pe.pinned=1;
  //      pe.exclude_kernel=1;
    //    pe.exclude_hv=1;
    //    pe.wakeup_events=1;
//	pe.precise_ip=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,-1,0,-1,0);
	if (fd<0) {
		if (!quiet) {
			fprintf(stderr,"Problem opening leader %s\n",
				strerror(errno));
			test_fail(test_string);
		}
	}
	our_mmap=mmap(NULL, mmap_pages*4096,
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
	ret=ioctl(fd, PERF_EVENT_IOC_DISABLE,0);

	if (!quiet) {
		printf("Counts %d, using mmap buffer %p\n",count_total,our_mmap);
	}

	if (count_total==0) {
		if (!quiet) printf("No overflow events generated.\n");
		test_fail(test_string);
	}
	munmap(our_mmap,mmap_pages*4096);

	close(fd);

	test_pass(test_string);

	return 0;
}
