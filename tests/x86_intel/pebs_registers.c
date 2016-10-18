/* This depends on a 3.17 kernel patched with */
/* Stephane Eranian's
   [PATCH v3 0/6] perf: add ability to sample interrupted machine state
*/

/* some notes */

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

	PEBS field contains

	RFLAGS/RIP/RAX/RBX/../R15

	BTS, regular overflow, and PEBS overflow share the same
	interrupt handler.

	Each processor has its own save area.

	18.4.2 repeats above info

	18.4.4 on core2 only 9 events support PEBS, and only
		available on one counter, PMC0
		ENABLE_PEBS bit in IA32_PEBS_ENABLE MSR

	THe PEBStrap bit allows trap-like (record state after instruction)
		of fault-like (record state before instruction)

	18.6.2 Atom Silvermont also supports PEBS

	18.7.1 Nehalem -- PEBS supported in all 4 general purpose counters
		Load latency support added.

		AnyThread, Edge, Invert, CMask must be zero in PEBS event

		Priority:  if simultaneous, event in PMC0 has precedence
		over PMC1, etc.

	18.7.1.2 Latench measure
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

	18.9.4.1 number of supported events increased

	With precise store, once triggered, full latency info
	captured on the next store that completes.

	MEM_TRANS_RETIRED.PRECISE_STORE

	address and status recorded

	18.9.4.4	PDIR
		Precise  Distribution of Instructions Retired

		Can avoid skid.  CPU notices when about to overflow
		and drops to more precise mode.

		Should quiet all other counters when using this(?)

		PDIR applies only to the INST_RETIRED.ALL precise event,
		and must use IA32_PMC1 with PerfEvtSel1 property
		configured and bit 1 in the IA32_PEBS_ENABLE set to 1.
		INST_RETIRED.ALL is a non-architectural performance
		event, it is not supported in prior generations.

	18.11	Haswell

		No precise store.

		Has transaction abort info

		Adds new "Eventing" field that has IP of instruction
		causing the event (as opposed to RIP which has the
		next instruction)

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

	18.13 = Pentium 4 PEBS

	18.13.7
		only one counter, only three events

***************
Linux interface
***************

	Does not support old 32-bit p4/core PEBS record format(?)

	Load latency, precise store, precise store haswell

	pebs_fixup_ip() ?
		makes fake Eventing info?
		pebs.trap?

	weight field holds pebs->lat


	To activate PEBS low-skid set to 1, 2, or 3

	To get register state,
		PERF_SAMPLE_REGS_USER

	To get latency values
		PERF_SAMPLE_WEIGHT

	To get data src
		PERF_SAMPLE_DATA_SRC

*/



/* TODO

   set precice_ip
   get register dump

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

#if defined(__x86_64__) || defined(__i386__) ||defined(__arm__)
#include "asm/perf_regs.h"
#endif

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
	int mmap_pages=1+MMAP_DATA_SIZE;

	struct perf_event_attr pe;

	struct sigaction sa;
	char test_string[]="Testing pebs...";

	quiet=test_quiet();

	if (!quiet) printf("This tests the intel PEBS interface.\n");

	        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_sigaction = our_handler;
        sa.sa_flags = SA_SIGINFO;

        if (sigaction( SIGIO, &sa, NULL) < 0) {
                fprintf(stderr,"Error setting up signal handler\n");
                exit(1);
        }

        /* Set up Instruction Event */

        memset(&pe,0,sizeof(struct perf_event_attr));

        pe.type=PERF_TYPE_HARDWARE;
        pe.size=sizeof(struct perf_event_attr);
        pe.config=PERF_COUNT_HW_INSTRUCTIONS;
        pe.sample_period=SAMPLE_FREQUENCY;
//        pe.sample_type=PERF_SAMPLE_IP | PERF_SAMPLE_REGS_USER;
        pe.sample_type=PERF_SAMPLE_IP | PERF_SAMPLE_REGS_INTR;

	global_sample_type=pe.sample_type;

#if defined(__i386__) || defined (__x86_64__)

	/* Bitfield saying which registers we want */
	pe.sample_regs_intr=(1ULL<<PERF_REG_X86_64_MAX)-1;
//	pe.sample_regs_user=(1ULL<<PERF_REG_X86_IP);
	/* DS, ES, FS, and GS not valid on x86_64 */
	/* see  perf_reg_validate() in arch/x86/kernel/perf_regs.c */
	pe.sample_regs_intr&=~(1ULL<<PERF_REG_X86_DS);
	pe.sample_regs_intr&=~(1ULL<<PERF_REG_X86_ES);
	pe.sample_regs_intr&=~(1ULL<<PERF_REG_X86_FS);
	pe.sample_regs_intr&=~(1ULL<<PERF_REG_X86_GS);


	printf("%llx %d\n",pe.sample_regs_user,PERF_REG_X86_DS);

#else
	pe.sample_regs_intr=1;
#endif

	global_sample_regs_user=pe.sample_regs_intr;

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
			if (errno==EINVAL) {
				fprintf(stderr,"Problem opening leader "
					"probably need to run a newer kernel: %s\n",
					strerror(errno));
			}
			else {
				fprintf(stderr,"Problem opening leader %s\n",
					strerror(errno));
			}
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
