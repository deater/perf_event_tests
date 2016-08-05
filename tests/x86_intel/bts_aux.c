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
#define AUX_DATA_SIZE 8

/* Global vars as I'm lazy */
static int count_total=0;

static char *our_mmap;
static char *our_aux;

static long sample_type;
static long read_format;
static int quiet;
static long long prev_head;


static void our_handler(int signum, siginfo_t *info, void *uc) {

	int ret;

	int fd = info->si_fd;

	ret=ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

	prev_head=perf_mmap_read(our_mmap,MMAP_DATA_SIZE,prev_head,
		sample_type,read_format,
		0, /* reg_mask */
		NULL, /*validate */
		quiet,
		NULL, /* events read */
		RAW_NONE);

	count_total++;

	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH, 1);

	(void) ret;

}

struct bts_record {
	long long branch_from;
	long long branch_to;
	long long predicted;

};

struct bts_record bts_buffer[AUX_DATA_SIZE*4096];

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

        /* Set up BTS Event */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.size=sizeof(struct perf_event_attr);
	pe.type=bts_type;
	pe.disabled=1;
//	pe.exclude_kernel=1;
//	pe.exclude_user=1;

	/* pe.config not necessary? */

//	sample_type=PERF_SAMPLE_IP|PERF_SAMPLE_WEIGHT|PERF_SAMPLE_ADDR;
//	read_format=0;
//	pe.sample_period=SAMPLE_FREQUENCY;
//	pe.sample_type=sample_type;
//	pe.read_format=read_format;
//	pe.pinned=1;
//	pe.exclude_hv=1;
//	pe.wakeup_events=1;
//	pe.precise_ip=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		if (!quiet) {
			fprintf(stderr,"Problem opening leader %s\n",
				strerror(errno));
			test_fail(test_string);
		}
	}

	/* Setup normal mmap page */

	our_mmap=mmap(NULL, mmap_pages*4096,
		PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	struct perf_event_mmap_page *mmap_control;

	mmap_control = (struct perf_event_mmap_page *)our_mmap;

	mmap_control->aux_offset=mmap_pages*4096;
	mmap_control->aux_size=AUX_DATA_SIZE*4096;

	/* Setup aux mmap page */

	our_aux=mmap(NULL, mmap_control->aux_size,
		PROT_READ|PROT_WRITE, MAP_SHARED, fd, mmap_control->aux_offset);


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

	ret=ioctl(fd, PERF_EVENT_IOC_DISABLE,0);

	printf("After, aux_head=%llx aux_tail=%llx\n",
		mmap_control->aux_head,mmap_control->aux_tail);

	/* Attempt to parse */

	memcpy(bts_buffer,our_aux,mmap_control->aux_head);

	int i;
	for(i=0;i< (mmap_control->aux_head/sizeof(struct bts_record));i++) {
		printf("From: %llx To: %llx Predicted: %d\n",
			bts_buffer[i].branch_from,
			bts_buffer[i].branch_to,
			!!(bts_buffer[i].predicted&0x10));
	}
	printf("\n");

	/* Clean up */

	munmap(our_aux,AUX_DATA_SIZE*4096);

	munmap(our_mmap,mmap_pages*4096);

	close(fd);

	test_pass(test_string);

	return 0;
}
