/*
	PERF_SAMPLE_WEIGHT and PERF_SAMPLE_DATA_SRC need at least 3.10 kernel

*/



/* Intel Volume 3-B

	17.4.9 -- Debug store area.
		On overflow, the PEBS event *after* the overflow
		records to the DS area the PEBS information
		The Instruction pointer points to one after
		the instruction causing the issue.

	Linux handles this in arch/x86/kernel/cpu/perf_event_intel_ds.c

	PEBS is configured in the DS area setup.  Sets min and max
	values of the memory reason, and what value to reset the
	counter to on interrupt.

	18.7.1 Nehalem -- PEBS supported in all 4 general purpose counters
		Load latency support added.

		AnyThread, Edge, Invert, CMask must be zero in PEBS event

	18.7.1.2 Latency measure
		MEM_INST_RETIRED event and LATENCY_ABOVE_THRESHOLD umask

		SR_PEBS_LD_LAT_THRESHOLD MSR programmed with the
		latency of interest, only above is counted.
		Minimum value for this is 3.

		Loads are randomly chosen to be tagged to measure
		latency info.  When PEBS is triggered, the most
		recent randomly tagged value is reported.

		Linear Address, Latency, and Source reported

	18.9.4 Sandybridge Support

		Like above but also stores.
		Stores in PMC3 only

		table 18-18

	With precise store, once triggered, full latency info
	captured on the next store that completes.


	18.11	Haswell

		No precise store.

		Precise store replaced by Data Address Profiling.

		MEM_UOPS_RETIRED.STLB_MISS_LOADS
		MEM_UOPS_RETIRED.STLB_MISS_STORES
		MEM_UOPS_RETIRED.LOCK_LOADS
		MEM_UOPS_RETIRED.SPLIT_STORES
		MEM_UOPS_RETIRED.SPLIT_LOADS
		MEM_UOPS_RETIRED.ALL_STORES
		MEM_UOPS_RETIRED.ALL_LOADS
		MEM_LOAD_UOPS_LLC_MISS_RETIRED.LOCAL_DRAM
		MEM_LOAD_UOPS_RETIRED.L1_HIT
		MEM_LOAD_UOPS_RETIRED.L2_HIT
		MEM_LOAD_UOPS_RETIRED.L3_HIT
		MEM_LOAD_UOPS_RETIRED.L1_MISS
		MEM_LOAD_UOPS_RETIRED.L2_MISS
		MEM_LOAD_UOPS_RETIRED.L3_MISS
		MEM_LOAD_UOPS_RETIRED.HIT_LFB
		MEM_LOAD_UOPS_L3_HIT_RETIRED.XSNP_MISS
		MEM_LOAD_UOPS_L3_HIT_RETIRED.XSNP_HIT
		MEM_LOAD_UOPS_L3_HIT_RETIRED.XSNP_HITM
		UOPS_RETIRED.ALL (if load or store is tagged)
		MEM_LOAD_UOPS_LLC_HIT_RETIRED.XSNP_NONE

		When enabled, the latency etc info is stored in the PEBS record

***************
Linux interface
***************

	Does not support old 32-bit p4/core PEBS record format(?)

	Load latency, precise store, precise store haswell

	pebs_fixup_ip() ?
		makes fake Eventing info?
		pebs.trap?

	weight field holds pebs->lat

	To get data src
		PERF_SAMPLE_DATA_SRC
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
#include "instructions_testcode.h"
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


int main(int argc, char **argv) {

	int ret;
	int fd;
	int mmap_pages=1+MMAP_DATA_SIZE;

	struct perf_event_attr pe;

	struct sigaction sa;
	char test_string[]="Testing pebs latency...";

	quiet=test_quiet();

	if (!quiet) printf("This tests the intel PEBS latency.\n");

	        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_sigaction = our_handler;
        sa.sa_flags = SA_SIGINFO;

        if (sigaction( SIGIO, &sa, NULL) < 0) {
                fprintf(stderr,"Error setting up signal handler\n");
                exit(1);
        }

        /* Set up Instruction Event */

        memset(&pe,0,sizeof(struct perf_event_attr));

	sample_type=PERF_SAMPLE_IP|PERF_SAMPLE_WEIGHT;
	read_format=0;

        pe.type=PERF_TYPE_HARDWARE;
        pe.size=sizeof(struct perf_event_attr);
        pe.config=PERF_COUNT_HW_INSTRUCTIONS;
        pe.sample_period=SAMPLE_FREQUENCY;
        pe.sample_type=sample_type;

        pe.read_format=read_format;
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

	instructions_million();

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

	test_pass(test_string);

	return 0;
}
