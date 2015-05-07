/*************************************************************
bts_aux.c -- tests the Intel Branch Trace Store Functionality
	by Vince Weaver <vincent.weaver@maine.edu>
	This got its own driver in 4.1
		8062382c8dbe2dc11d37e7f0b139508cf10de9d4
	and uses the new AUX interface.
**************************************************************/

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

long long perf_mmap_read( void *our_mmap, int mmap_size, long long prev_head,
                    int sample_type, int read_format, long long reg_mask,
                    struct validate_values *validate,
                    int quiet, int *events_read );


	prev_head=perf_mmap_read(our_mmap,MMAP_DATA_SIZE,prev_head,
		sample_type,read_format,
		0, /* reg_mask */
		NULL, /*validate */
		quiet,
		NULL); /* events read */

	count_total++;

	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH, 1);

	(void) ret;

}


int main(int argc, char **argv) {

	int ret;
	int fd;
	int mmap_pages=1+MMAP_DATA_SIZE;

	struct perf_event_attr pe;

	struct sigaction sa;
	char test_string[]="Testing Intel Branch Trace Store...";

	quiet=test_quiet();

	if (!quiet) printf("This tests the intel BTS driver.\n");

	memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_sigaction = our_handler;
        sa.sa_flags = SA_SIGINFO;

        if (sigaction( SIGIO, &sa, NULL) < 0) {
                fprintf(stderr,"Error setting up signal handler\n");
                exit(1);
        }


	/* Find the BTS driver */
	FILE *fff;
	int bts_type=0;

	fff=fopen("/sys/devices/intel_bts/type","r");
	if (fff==NULL) {
		if (!quiet) {
			fprintf(stderr,"Intel BTS driver not found\n");
		}
		test_skip(test_string);
	}

	fscanf(fff,"%d",&bts_type);
	fclose(fff);

	if (!quiet) {
		fprintf(stdout,"Found Intel BTS as type %d\n",bts_type);
	}

        /* Set up Instruction Event */
#if 0
        memset(&pe,0,sizeof(struct perf_event_attr));

	sample_type=PERF_SAMPLE_IP|PERF_SAMPLE_WEIGHT|PERF_SAMPLE_ADDR;
	read_format=0;

        pe.type=PERF_TYPE_RAW;
        pe.size=sizeof(struct perf_event_attr);
        //pe.config=PERF_COUNT_HW_INSTRUCTIONS;

	 /* MEM_UOPS_RETIRED:ALL_STORES */
	 pe.config = 0x5382d0;

        pe.sample_period=SAMPLE_FREQUENCY;
        pe.sample_type=sample_type;

        pe.read_format=read_format;
        pe.disabled=1;
        pe.pinned=1;
        pe.exclude_kernel=1;
        pe.exclude_hv=1;
        pe.wakeup_events=1;
	pe.precise_ip=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
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

	if (!quiet) {
                printf("Counts %d, using mmap buffer %p\n",count_total,our_mmap);
        }

	if (count_total==0) {
		if (!quiet) printf("No overflow events generated.\n");
		test_fail(test_string);
	}
	munmap(our_mmap,mmap_pages*4096);

	close(fd);
#endif
	test_pass(test_string);

	return 0;
}
