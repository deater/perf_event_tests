/* Fixed in 26e61e8939b1fe8729572dabe9a9e97d930dd4f6    */
/* 3.14-rc5                                             */

/* On a core2 machine from ?? until 3.14-rc5 		*/
/* The code generates a warning like this:		*/
/*
[   47.296031] ------------[ cut here ]------------
[   47.300013] WARNING: CPU: 0 PID: 2821 at arch/x86/kernel/cpu/perf_event.c:1076 x86_pmu_start+0x46/0xee()
[   47.300013] Modules linked in: cpufreq_userspace cpufreq_stats cpufreq_powersave cpufreq_conservative f71882fg mcs7830 usbnet evdev ohci_pci ohci_hcd pcspkr i2c_nforce2 psmouse serio_raw coretemp video wmi button acpi_cpufreq processor thermal_sys ehci_pci ehci_hcd sg sd_mod usbcore usb_common
[   47.300013] CPU: 0 PID: 2821 Comm: out Not tainted 3.14.0-rc2 #2
[   47.300013] Hardware name: AOpen   DE7000/nMCP7ALPx-DE R1.06 Oct.19.2012, BIOS 080015  10/19/2012
[   47.300013]  0000000000000000 ffffffff817f25ad ffffffff814e746b 0000000000000000
[   47.300013]  ffffffff8103bf1c ffff880037e62420 ffffffff81012126 ffff88011fc13400
[   47.300013]  ffff88011b2b1800 ffff88011fc0b940 0000000000000021 0000000000000000
[   47.300013] Call Trace:
[   47.300013]  [<ffffffff814e746b>] ? dump_stack+0x41/0x56
[   47.300013]  [<ffffffff8103bf1c>] ? warn_slowpath_common+0x79/0x92
[   47.300013]  [<ffffffff81012126>] ? x86_pmu_start+0x46/0xee
[   47.300013]  [<ffffffff81012126>] ? x86_pmu_start+0x46/0xee
[   47.300013]  [<ffffffff810123a1>] ? x86_pmu_enable+0x1d3/0x285
[   47.300013]  [<ffffffff810bca0d>] ? perf_event_context_sched_in+0x6d/0x8d
[   47.300013]  [<ffffffff810bca4e>] ? __perf_event_task_sched_in+0x21/0x108
[   47.300013]  [<ffffffff810666bc>] ? idle_balance+0x11a/0x157
[   47.300013]  [<ffffffff810628f5>] ? finish_task_switch+0x40/0xa5
[   47.300013]  [<ffffffff814e7e45>] ? __schedule+0x46a/0x4bd
[   47.300013]  [<ffffffff814e749d>] ? schedule_timeout+0x1d/0xb4
[   47.300013]  [<ffffffff8108bba6>] ? generic_exec_single+0x3f/0x52
[   47.300013]  [<ffffffff8102f017>] ? perf_reg_value+0x4c/0x4c
[   47.300013]  [<ffffffff8108bd33>] ? smp_call_function_single+0xdc/0xf2
[   47.300013]  [<ffffffff814e7480>] ? dump_stack+0x56/0x56
[   47.300013]  [<ffffffff814e8587>] ? __wait_for_common+0xce/0x14a
[   47.300013]  [<ffffffff8106286d>] ? try_to_wake_up+0x19a/0x19a
[   47.300013]  [<ffffffff810a1b47>] ? get_tracepoint+0x20/0x53
[   47.300013]  [<ffffffff8107ea47>] ? T.944+0x1c8/0x1c8
[   47.300013]  [<ffffffff8107c9f9>] ? wait_rcu_gp+0x3f/0x46
[   47.300013]  [<ffffffff8107ca00>] ? wait_rcu_gp+0x46/0x46
[   47.300013]  [<ffffffff810b2014>] ? perf_trace_event_unreg+0x2e/0xbd
[   47.300013]  [<ffffffff810b20d1>] ? perf_trace_destroy+0x2e/0x3b
[   47.300013]  [<ffffffff810bc0bf>] ? __free_event+0x2d/0x52
[   47.300013]  [<ffffffff810bfdec>] ? perf_event_release_kernel+0x74/0x7b
[   47.300013]  [<ffffffff810bfe9d>] ? perf_release+0x10/0x14
[   47.300013]  [<ffffffff81105b03>] ? __fput+0xdf/0x1a4
[   47.300013]  [<ffffffff810538d9>] ? task_work_run+0x7f/0x96
[   47.300013]  [<ffffffff814f09f0>] ? int_signal+0x12/0x17
[   47.300013] ---[ end trace 9ccad2f02057baa8 ]---
*/

/* Note, fails to run on ivb because of the precise=1 value in fd[23] */

/* The WARN_ONCE was introduced in 2.6.37 */
/* The relevant line number moved around a lot. */
/*	2.6.37 = 1035	*/
/*	2.6.38 = 1058	*/
/*	2.6.39 = 1152	*/
/*	3.0  = 1159	*/
/*	3.1  = 1173	*/
/*	3.2  =  860	*/
/*	3.3  =  989	*/
/*	3.4  = 1054	*/
/*	3.5  = 1051	*/
/*	3.6  = 1046	*/
/*	3.7  = 1054	*/
/*	3.8  = 1048	*/
/*	3.9  = 1048	*/
/*	3.10 = 1054	*/
/*	3.11 = 1076	*/
/*	3.12 = 1076	*/
/*	3.13 = 1076	*/


/* Found with perf_fuzzer */

/* The warning was triggered in another manner in February 2012		*/
/*   https://lkml.org/lkml/2012/2/2/453					*/

/* Reported to trinity list 23 May 2013					*/
/*   http://www.spinics.net/lists/trinity/msg00087.html			*/
/* Reported as part of other problem, 30 July 2013			*/
/*   https://lkml.org/lkml/2013/7/30/666				*/
/* Reported as part of other issue, 13 November 2013			*/
/*   https://lkml.org/lkml/2013/11/13/472				*/
/* Final report of issue, Dave Jones, 30 January 2014 using fuzzer 	*/
/*   https://lkml.org/lkml/2014/1/30/355				*/


/* by Vince Weaver <vincent.weaver _at_ maine.edu */

#define _GNU_SOURCE 1
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <poll.h>

#ifdef __sparc__
#define HW_BREAKPOINT_EMPTY	0
#define HW_BREAKPOINT_R	1
#define HW_BREAKPOINT_W 2
#else
#include <linux/hw_breakpoint.h>
#endif

#include "perf_event.h"

int fd[1024];
struct perf_event_attr pe[1024];

int perf_event_open(struct perf_event_attr *hw_event_uptr,
	pid_t pid, int cpu, int group_fd, unsigned long flags) {

	return syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,
		group_fd, flags);
}

int main(int argc, char **argv) {

	int i;

	for(i=0;i<1024;i++) fd[i]=-1;

/* 1 */
/* Random Seed was 1392048997 */
/* 2 */
/* 3 */
/* /proc/sys/kernel/perf_event_max_sample_rate was 100000 */
/* 4 */
/* fd = 3 */

	memset(&pe[3],0,sizeof(struct perf_event_attr));
	pe[3].type=PERF_TYPE_BREAKPOINT;
	pe[3].size=72;
	pe[3].config=0xd9ab819b;
	pe[3].sample_type=0; /* 0 */
	pe[3].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING|PERF_FORMAT_ID; /* 7 */
	pe[3].inherit=1;
	pe[3].exclude_kernel=1;
	pe[3].exclude_hv=1;
	pe[3].exclude_idle=1;
	pe[3].freq=1;
	pe[3].inherit_stat=1;
	pe[3].enable_on_exec=1;
	pe[3].precise_ip=1; /* constant skid */
	pe[3].sample_id_all=1;
	pe[3].wakeup_events=0;
	pe[3].bp_type=HW_BREAKPOINT_R|HW_BREAKPOINT_W; /*3*/
	pe[3].bp_addr=0x0;
	pe[3].bp_len=0x8;

	fd[3]=perf_event_open(&pe[3],
				getpid(), /* current thread */
				-1, /* all cpus */
				fd[19], /* 19 is group leader */
				PERF_FLAG_FD_NO_GROUP /*1*/ );


/* 5 */
/* fd = 23 */

	memset(&pe[23],0,sizeof(struct perf_event_attr));
	pe[23].type=PERF_TYPE_HARDWARE;
	pe[23].config=PERF_COUNT_HW_INSTRUCTIONS;
	pe[23].sample_type=0; /* 0 */
	pe[23].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED; /* 1 */
	pe[23].exclude_hv=1;
	pe[23].exclude_idle=1;
	pe[23].mmap=1;
	pe[23].enable_on_exec=1;
	pe[23].task=1;
	pe[23].watermark=1;
	pe[23].precise_ip=1; /* constant skid */
	pe[23].mmap_data=1;
	pe[23].sample_id_all=1;
	pe[23].exclude_host=1;
	pe[23].exclude_guest=1;
	pe[23].exclude_callchain_user=1;
	pe[23].wakeup_watermark=0;
	pe[23].bp_type=HW_BREAKPOINT_EMPTY;

	fd[23]=perf_event_open(&pe[23],
				0, /* current thread */
				-1, /* all cpus */
				fd[15], /* 15 is group leader */
				0 /*0*/ );


/* 6 */
/* fd = 4 */

	memset(&pe[4],0,sizeof(struct perf_event_attr));
	pe[4].type=PERF_TYPE_HARDWARE;
	pe[4].config=PERF_COUNT_HW_CPU_CYCLES;
	pe[4].sample_period=0xb32acea2;
	pe[4].sample_type=PERF_SAMPLE_DATA_SRC; /* 8000 */
	pe[4].read_format=PERF_FORMAT_TOTAL_TIME_RUNNING; /* 2 */
	pe[4].exclude_user=1;
	pe[4].exclude_kernel=1;
	pe[4].exclude_hv=1;
	pe[4].exclude_idle=1;
	pe[4].mmap=1;
	pe[4].comm=1;
	pe[4].inherit_stat=1;
	pe[4].precise_ip=0; /* arbitrary skid */
	pe[4].sample_id_all=1;
	pe[4].exclude_host=1;
	pe[4].exclude_guest=1;
	pe[4].exclude_callchain_kernel=1;
	pe[4].wakeup_events=255;
	pe[4].bp_type=HW_BREAKPOINT_EMPTY;
	pe[4].branch_sample_type=PERF_SAMPLE_BRANCH_ANY_CALL|PERF_SAMPLE_BRANCH_ANY_RETURN|PERF_SAMPLE_BRANCH_IND_CALL;
	pe[4].sample_regs_user=12;
	pe[4].sample_stack_user=10497;

	fd[4]=perf_event_open(&pe[4],
				0, /* current thread */
				-1, /* all cpus */
				fd[3], /* 3 is group leader */
				0 /*0*/ );


/* 7 */
/* fd = 5 */

	memset(&pe[5],0,sizeof(struct perf_event_attr));
	pe[5].type=PERF_TYPE_TRACEPOINT;
	pe[5].size=96;
	pe[5].config=0x1d; /* 29 irq_vectors/error_apic_entry */
	pe[5].sample_type=0; /* 0 */
	pe[5].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_ID; /* 5 */
	pe[5].disabled=1;
	pe[5].inherit=1;
	pe[5].exclusive=1;
	pe[5].exclude_kernel=1;
	pe[5].exclude_hv=1;
	pe[5].comm=1;
	pe[5].freq=1;
	pe[5].inherit_stat=1;
	pe[5].enable_on_exec=1;
	pe[5].watermark=1;
	pe[5].precise_ip=1; /* constant skid */
	pe[5].exclude_host=1;
	pe[5].exclude_callchain_user=1;
	pe[5].wakeup_watermark=0;
	pe[5].bp_type=HW_BREAKPOINT_EMPTY;
	pe[5].config1=0x7;

	fd[5]=perf_event_open(&pe[5],
				0, /* current thread */
				-1, /* all cpus */
				-1, /* New Group Leader */
				0 /*0*/ );


/* 8 */
/* fd = 15 */

	memset(&pe[15],0,sizeof(struct perf_event_attr));
	pe[15].type=PERF_TYPE_HARDWARE;
	pe[15].config=PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
	pe[15].sample_type=0; /* 0 */
	pe[15].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_ID; /* 5 */
	pe[15].exclude_hv=1;
	pe[15].exclude_idle=1;
	pe[15].mmap=1;
	pe[15].freq=1;
	pe[15].inherit_stat=1;
	pe[15].enable_on_exec=1;
	pe[15].precise_ip=0; /* arbitrary skid */
	pe[15].wakeup_events=0;
	pe[15].bp_type=HW_BREAKPOINT_EMPTY;

	fd[15]=perf_event_open(&pe[15],
				getpid(), /* current thread */
				-1, /* all cpus */
				fd[3], /* 3 is group leader */
				0 /*0*/ );


/* 9 */
	close(fd[5]);

	/* Replayed 9 syscalls */
	return 0;
}
