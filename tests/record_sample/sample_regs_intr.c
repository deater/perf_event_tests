/* sample_regs_intr.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* An attempt to figure out the PERF_SAMPLE_REGS_INTR code */
/* Added in Linux 3.19		*/

/* Its primary difference between PERF_SAMPLE_REGS_USER */
/* is that it can return the registers from inside the kernel. */


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
	char test_string[]="Testing PERF_SAMPLE_REGS_INTR...";

	quiet=test_quiet();

	if (!quiet) printf("This tests PERF_SAMPLE_REGS_INTR samples\n");

	memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_sigaction = our_handler;
        sa.sa_flags = SA_SIGINFO;

        if (sigaction( SIGIO, &sa, NULL) < 0) {
                fprintf(stderr,"Error setting up signal handler\n");
                exit(1);
        }

	memset(&pe,0,sizeof(struct perf_event_attr));
        pe.size=sizeof(struct perf_event_attr);
        pe.sample_period=SAMPLE_FREQUENCY;
        pe.sample_type=PERF_SAMPLE_IP | PERF_SAMPLE_REGS_USER | PERF_SAMPLE_REGS_INTR;
	global_sample_type=pe.sample_type;

#if defined (__x86_64__)

	/* Bitfield saying which registers we want */
	pe.sample_regs_user=(1ULL<<PERF_REG_X86_64_MAX)-1;
	/* DS, ES, FS, and GS not valid on x86_64 */
	/* see  perf_reg_validate() in arch/x86/kernel/perf_regs.c */
	pe.sample_regs_user&=~(1ULL<<PERF_REG_X86_DS);
	pe.sample_regs_user&=~(1ULL<<PERF_REG_X86_ES);
	pe.sample_regs_user&=~(1ULL<<PERF_REG_X86_FS);
	pe.sample_regs_user&=~(1ULL<<PERF_REG_X86_GS);

	pe.sample_regs_intr=pe.sample_regs_user;

        pe.read_format=0;
        pe.disabled=1;
        pe.pinned=1;
        pe.wakeup_events=1;


#elif defined(__i386__)
	pe.sample_regs_user=(1ULL<<PERF_REG_X86_32_MAX)-1;
	pe.sample_regs_intr=pe.sample_regs_user;
#elif defined(__arm__)
	pe.sample_regs_user=(1ULL<<PERF_REG_ARM_MAX)-1;
	pe.sample_regs_intr=pe.sample_regs_user;
#else
	pe.sample_regs_user=1;
	pe.sample_regs_intr=1;
#endif

	global_sample_regs_user=pe.sample_regs_user;


	if (detect_vendor()==VENDOR_AMD) {
		if (!quiet) printf("Using cycles:pp on AMD\n");
		/* On AMD cycles is a precise event */
		pe.type=PERF_TYPE_HARDWARE;
		pe.config=PERF_COUNT_HW_CPU_CYCLES;

		/* on AMD ibs the following must be false */
		/* see bad9ac2d7f878a31cf1ae8c1ee3768077d222bcb */
		/*	.exclude_user   = 1,
		.exclude_kernel = 1,
		.exclude_hv     = 1,
		.exclude_idle   = 1,
		.exclude_host   = 1,
		.exclude_guest  = 1,
		*/

	        pe.exclude_user    = 0;
        	pe.exclude_kernel  = 0;
        	pe.exclude_hv	   = 0;
		pe.exclude_idle	   = 0;
		pe.exclude_host	   = 0;
		pe.exclude_guest   = 0;

	}

	else {
		if (!quiet) printf("Using instructions:pp\n");
		/* Set up Instruction Event */
		//	pe.type=PERF_TYPE_RAW;
		//	pe.config=0x5300c0; // INST_RETIRED:ANY_P
		pe.type=PERF_TYPE_HARDWARE;
		pe.config=PERF_COUNT_HW_INSTRUCTIONS;

		pe.exclude_kernel=0;
		pe.exclude_hv=1;
	}

	arch_adjust_domain(&pe,quiet);

	/* Must be greater than 0 for sample_regs_intr to be interesting? */
	/* Not seeing any difference. */
	pe.precise_ip=2;

	if (detect_vendor()==VENDOR_AMD) {
		/* On AMD needs to be system-wide per-cpu event */
		/* or the IBS PMU won't work.			*/
		fd=perf_event_open(&pe,-1,0,-1,0);
	}
	else {
		fd=perf_event_open(&pe,0,-1,-1,0);
	}
	if (fd<0) {
		if (!quiet) {
			fprintf(stderr,"Problem opening leader %s\n",
				strerror(errno));
		}
		if (errno==EACCES) {
			test_skip(test_string);
		}
		test_fail(test_string);
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

	int i;
	for(i=0;i<10;i++) {
//		write(1,"0",1);
		instructions_million();
	}

	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH,0);

	if (!quiet) {
                printf("Counts %d, using mmap buffer %p\n",count_total,our_mmap);
        }

	if (count_total==0) {
		if (!quiet) printf("No overflow events generated.\n");
		test_fail(test_string);
	}

	/* Just disabling does not turn off the signal handler */
	/* Well it does, but before it can finish it calls the signal */
	/* handler one last time which disables/refreshes */
	/* Oddly we only see this issue on amdfam15h */

	/* Disable signal handler */
        sa.sa_handler = SIG_IGN;
        if (sigaction( SIGIO, &sa, NULL) < 0) {
                fprintf(stderr,"Error setting up signal handler\n");
                exit(1);
        }

	ret=ioctl(fd, PERF_EVENT_IOC_DISABLE,0);

	munmap(our_mmap,mmap_pages*getpagesize());

	close(fd);

	test_pass(test_string);

	return 0;
}
