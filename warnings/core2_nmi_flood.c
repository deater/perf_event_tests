/* core2_nmi_flood.c					*/
/* by Vince Weaver <vincent.weaver _at_ maine.edu	*/

/* This code, when run in a tight loop, i.e.		*/
/*	while true; do ./core2_nmi_flood ; done		*/
/* will lead to lots of NMI messages printed to the	*/
/* syslog on a core2 machine running recent (3.12)	*/
/* Linux.						*/

/*
[ 7911.810186] Uhhuh. NMI received for unknown reason 3d on CPU 0.
[ 7911.812756] Do you have a strange power saving mode enabled?
[ 7911.812756] Dazed and confused, but trying to continue
[ 7912.554225] Uhhuh. NMI received for unknown reason 2d on CPU 1.
[ 7912.556758] Do you have a strange power saving mode enabled?
[ 7912.556758] Dazed and confused, but trying to continue
[ 7912.838169] Uhhuh. NMI received for unknown reason 2d on CPU 0.
[ 7912.840726] Do you have a strange power saving mode enabled?
[ 7912.840726] Dazed and confused, but trying to continue
*/

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
#include <poll.h>

#ifdef __sparc__
#define HW_BREAKPOINT_EMPTY 0
#define HW_BREAKPOINT_W	1
#else
#include <linux/hw_breakpoint.h>
#endif

#include "perf_event.h"

int fd[1024];
struct perf_event_attr pe[1024];


long long id;

struct sigaction sa;
static int overflows=0;

static void our_handler(int signum, siginfo_t *info, void *uc) {
	int fd = info->si_fd;


	ioctl(fd,PERF_EVENT_IOC_DISABLE,0);
	overflows++;
	ioctl(fd, PERF_EVENT_IOC_REFRESH,1);
}
int perf_event_open(struct perf_event_attr *hw_event_uptr,
	pid_t pid, int cpu, int group_fd, unsigned long flags) {

	return syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,
		group_fd, flags);
}

int main(int argc, char **argv) {
/* 1 */
/* 2 */
/* 3 */
/* fd = 15 */

	memset(&pe[15],0,sizeof(struct perf_event_attr));
	pe[15].type=PERF_TYPE_HARDWARE;
	pe[15].size=96;
	pe[15].config=PERF_COUNT_HW_CACHE_REFERENCES;
	pe[15].sample_period=0xf2;
	pe[15].sample_type=PERF_SAMPLE_TID|PERF_SAMPLE_ADDR|PERF_SAMPLE_READ|PERF_SAMPLE_CALLCHAIN|PERF_SAMPLE_RAW|PERF_SAMPLE_STACK_USER|PERF_SAMPLE_WEIGHT;//|PERF_SAMPLE_IDENTIFIER; /* 1643a */
	pe[15].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING|PERF_FORMAT_ID; /* 7 */
	pe[15].disabled=1;
	pe[15].pinned=1;
	pe[15].exclude_idle=1;
	pe[15].mmap=1;
	pe[15].inherit_stat=1;
	pe[15].precise_ip=0; /* arbitrary skid */
	pe[15].exclude_guest=1;
	pe[15].exclude_callchain_kernel=1;
	pe[15].exclude_callchain_user=1;
	pe[15].wakeup_events=-1037221357;
	pe[15].bp_type=HW_BREAKPOINT_EMPTY;
	pe[15].config2=0xffffffffffb24240;
	pe[15].branch_sample_type=PERF_SAMPLE_BRANCH_USER|PERF_SAMPLE_BRANCH_KERNEL|PERF_SAMPLE_BRANCH_HV|PERF_SAMPLE_BRANCH_ANY;//|PERF_SAMPLE_BRANCH_ABORT_TX|0x8101000000000000ULL;
	pe[15].sample_regs_user=-2130706432;
	pe[15].sample_stack_user=176;

	fd[15]=perf_event_open(&pe[15],
				0, /* current thread */
				-1, /* all cpus */
				-1, /* New Group Leader */
				0 /*0*/ );


/* 4 */
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;
	if (sigaction( SIGRTMIN+2, &sa, NULL) < 0) {
		printf("Error setting up signal handler\n");
	}
	fcntl(fd[15], F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd[15], F_SETSIG, SIGRTMIN+2);
	fcntl(fd[15], F_SETOWN,getpid());
/* 5 */
/* fd = 16 */

	memset(&pe[16],0,sizeof(struct perf_event_attr));
	pe[16].type=PERF_TYPE_SOFTWARE;
	pe[16].config=PERF_COUNT_SW_TASK_CLOCK;
	pe[16].sample_type=0; /* 0 */
	pe[16].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_GROUP; /* 9 */
	pe[16].disabled=1;
	pe[16].exclusive=1;
	pe[16].exclude_user=1;
	pe[16].exclude_kernel=1;
	pe[16].exclude_hv=1;
	pe[16].task=1;
	pe[16].precise_ip=0; /* arbitrary skid */
	pe[16].mmap_data=1;
	pe[16].sample_id_all=1;
	pe[16].exclude_host=1;
	pe[16].exclude_guest=1;
	pe[16].wakeup_events=0;
	pe[16].bp_type=HW_BREAKPOINT_EMPTY;

	fd[16]=perf_event_open(&pe[16],
				0, /* current thread */
				-1, /* all cpus */
				fd[11], /* 11 is group leader */
				PERF_FLAG_FD_NO_GROUP /*1*/ );


/* 6 */
	close(fd[11]);
/* 7 */
/* fd = 11 */

	memset(&pe[11],0,sizeof(struct perf_event_attr));
	pe[11].type=PERF_TYPE_HW_CACHE;
	pe[11].config=PERF_COUNT_HW_CACHE_LL | ( PERF_COUNT_HW_CACHE_OP_READ << 8) | ( PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16 );
	pe[11].sample_type=0; /* 0 */
	pe[11].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_ID; /* 5 */
	pe[11].inherit=1;
	pe[11].exclusive=1;
	pe[11].exclude_hv=1;
	pe[11].comm=1;
	pe[11].freq=1;
	pe[11].enable_on_exec=1;
	pe[11].precise_ip=0; /* arbitrary skid */
	pe[11].sample_id_all=1;
	pe[11].exclude_guest=1;
	pe[11].exclude_callchain_user=1;
	pe[11].wakeup_events=0;
	pe[11].bp_type=HW_BREAKPOINT_EMPTY;

	fd[11]=perf_event_open(&pe[11],
				0, /* current thread */
				-1, /* all cpus */
				-1, /* New Group Leader */
				PERF_FLAG_FD_OUTPUT /*2*/ );


/* 8 */
/* fd = 17 */

	memset(&pe[17],0,sizeof(struct perf_event_attr));
	pe[17].type=PERF_TYPE_SOFTWARE;
	pe[17].size=80;
	pe[17].config=PERF_COUNT_SW_TASK_CLOCK;
	pe[17].sample_type=0; /* 0 */
	pe[17].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_ID; /* 5 */
	pe[17].disabled=1;
	pe[17].inherit=1;
	pe[17].exclude_idle=1;
	pe[17].comm=1;
	pe[17].freq=1;
	pe[17].inherit_stat=1;
	pe[17].enable_on_exec=1;
	pe[17].precise_ip=3; /* must have zero skid */
	pe[17].mmap_data=1;
	pe[17].sample_id_all=1;
	pe[17].exclude_guest=1;
	pe[17].exclude_callchain_kernel=1;
	pe[17].wakeup_events=0;
	pe[17].bp_type=HW_BREAKPOINT_EMPTY;

	fd[17]=perf_event_open(&pe[17],
				0, /* current thread */
				-1, /* all cpus */
				-1, /* New Group Leader */
				PERF_FLAG_FD_NO_GROUP /*1*/ );


/* 9 */
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;
	if (sigaction( SIGRTMIN+2, &sa, NULL) < 0) {
		printf("Error setting up signal handler\n");
	}
	fcntl(fd[17], F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd[17], F_SETSIG, SIGRTMIN+2);
	fcntl(fd[17], F_SETOWN,getpid());
/* 10 */
/* fd = 4 */

	memset(&pe[4],0,sizeof(struct perf_event_attr));
	pe[4].type=PERF_TYPE_TRACEPOINT;
	pe[4].size=96;
	pe[4].config=0x32;
	pe[4].sample_period=0xffffffffffffffff;
	pe[4].sample_type=PERF_SAMPLE_IP|PERF_SAMPLE_TIME|PERF_SAMPLE_READ|PERF_SAMPLE_STREAM_ID|PERF_SAMPLE_WEIGHT; /* 4215 */
	pe[4].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING|PERF_FORMAT_ID|PERF_FORMAT_GROUP; /* f */
	pe[4].comm=1;
	pe[4].inherit_stat=1;
	pe[4].enable_on_exec=1;
	pe[4].task=1;
	pe[4].watermark=1;
	pe[4].precise_ip=1; /* constant skid */
	pe[4].mmap_data=1;
	pe[4].exclude_guest=1;
	pe[4].exclude_callchain_user=1;
	pe[4].wakeup_watermark=15465631;
	pe[4].bp_type=HW_BREAKPOINT_W; /*2*/
	pe[4].bp_addr=0xffff7e4b9558b75d;
	pe[4].bp_len=0x4;
	pe[4].branch_sample_type=PERF_SAMPLE_BRANCH_USER|PERF_SAMPLE_BRANCH_KERNEL|PERF_SAMPLE_BRANCH_HV|PERF_SAMPLE_BRANCH_ANY|PERF_SAMPLE_BRANCH_ANY_CALL|PERF_SAMPLE_BRANCH_ANY_RETURN|PERF_SAMPLE_BRANCH_IND_CALL;//|PERF_SAMPLE_BRANCH_ABORT_TX|PERF_SAMPLE_BRANCH_IN_TX|PERF_SAMPLE_BRANCH_NO_TX|0x8400000000006c00ULL;
	pe[4].sample_regs_user=-2918402996084736013;
	pe[4].sample_stack_user=956639924;

	fd[4]=perf_event_open(&pe[4],
				0, /* current thread */
				-1, /* all cpus */
				-1, /* New Group Leader */
				PERF_FLAG_FD_NO_GROUP|PERF_FLAG_FD_OUTPUT /*3*/ );


/* 11 */
/* fd = 5 */

	memset(&pe[5],0,sizeof(struct perf_event_attr));
	pe[5].type=PERF_TYPE_HARDWARE;
	pe[5].size=96;
	pe[5].config=PERF_COUNT_HW_BRANCH_MISSES;
	pe[5].sample_type=0; /* 0 */
	pe[5].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING|PERF_FORMAT_ID; /* 7 */
	pe[5].disabled=1;
	pe[5].inherit=1;
	pe[5].exclusive=1;
	pe[5].exclude_user=1;
	pe[5].exclude_kernel=1;
	pe[5].exclude_idle=1;
	pe[5].mmap=1;
	pe[5].comm=1;
	pe[5].freq=1;
	pe[5].enable_on_exec=1;
	pe[5].precise_ip=0; /* arbitrary skid */
	pe[5].mmap_data=1;
	pe[5].exclude_host=1;
	pe[5].wakeup_events=0;
	pe[5].bp_type=HW_BREAKPOINT_EMPTY;

	fd[5]=perf_event_open(&pe[5],
				0, /* current thread */
				-1, /* all cpus */
				-1, /* New Group Leader */
				PERF_FLAG_FD_OUTPUT /*2*/ );


/* 12 */
/* fd = 6 */

	memset(&pe[6],0,sizeof(struct perf_event_attr));
	pe[6].type=PERF_TYPE_RAW;
	pe[6].size=72;
	pe[6].config=0x808000004240580a;
	pe[6].sample_type=0; /* 0 */
	pe[6].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING; /* 3 */
	pe[6].inherit=1;
	pe[6].exclusive=1;
	pe[6].exclude_hv=1;
	pe[6].mmap=1;
	pe[6].freq=1;
	pe[6].task=1;
	pe[6].watermark=1;
	pe[6].precise_ip=0; /* arbitrary skid */
	pe[6].sample_id_all=1;
	pe[6].exclude_host=1;
	pe[6].exclude_guest=1;
	pe[6].exclude_callchain_kernel=1;
	pe[6].wakeup_watermark=0;
	pe[6].bp_type=HW_BREAKPOINT_EMPTY;

	fd[6]=perf_event_open(&pe[6],
				0, /* current thread */
				-1, /* all cpus */
				-1, /* New Group Leader */
				PERF_FLAG_FD_NO_GROUP|PERF_FLAG_FD_OUTPUT /*3*/ );


/* 13 */
	prctl(PR_TASK_PERF_EVENTS_ENABLE);
/* 14 */
/* fd = 8 */

	memset(&pe[8],0,sizeof(struct perf_event_attr));
	pe[8].type=PERF_TYPE_RAW;
	pe[8].config=0x3c21;
	pe[8].sample_type=0; /* 0 */
	pe[8].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING; /* 3 */
	pe[8].disabled=1;
	pe[8].exclude_hv=1;
	pe[8].mmap=1;
	pe[8].comm=1;
	pe[8].inherit_stat=1;
	pe[8].task=1;
	pe[8].precise_ip=0; /* arbitrary skid */
	pe[8].exclude_host=1;
	pe[8].exclude_callchain_kernel=1;
	pe[8].wakeup_events=0;
	pe[8].bp_type=HW_BREAKPOINT_EMPTY;

	fd[8]=perf_event_open(&pe[8],
				0, /* current thread */
				-1, /* all cpus */
				fd[18], /* 18 is group leader */
				PERF_FLAG_FD_NO_GROUP /*1*/ );


/* 15 */
	prctl(PR_TASK_PERF_EVENTS_DISABLE);
/* 16 */
/* fd = 20 */

	memset(&pe[20],0,sizeof(struct perf_event_attr));
	pe[20].type=PERF_TYPE_RAW;
	pe[20].config=0xc0ffffffe65b1a09;
	pe[20].sample_type=0; /* 0 */
	pe[20].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING; /* 3 */
	pe[20].exclusive=1;
	pe[20].exclude_user=1;
	pe[20].exclude_kernel=1;
	pe[20].exclude_hv=1;
	pe[20].mmap=1;
	pe[20].comm=1;
	pe[20].watermark=1;
	pe[20].precise_ip=0; /* arbitrary skid */
	pe[20].sample_id_all=1;
	pe[20].exclude_host=1;
	pe[20].exclude_guest=1;
	pe[20].exclude_callchain_kernel=1;
	pe[20].wakeup_watermark=0;
	pe[20].bp_type=HW_BREAKPOINT_EMPTY;

	fd[20]=perf_event_open(&pe[20],
				0, /* current thread */
				-1, /* all cpus */
				-1, /* New Group Leader */
				PERF_FLAG_FD_NO_GROUP /*1*/ );


/* 17 */
/* fd = 21 */

	memset(&pe[21],0,sizeof(struct perf_event_attr));
	pe[21].type=PERF_TYPE_SOFTWARE;
	pe[21].config=PERF_COUNT_SW_TASK_CLOCK;
	pe[21].sample_type=0; /* 0 */
	pe[21].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_GROUP; /* 9 */
	pe[21].pinned=1;
	pe[21].exclude_user=1;
	pe[21].exclude_idle=1;
	pe[21].inherit_stat=1;
	pe[21].enable_on_exec=1;
	pe[21].watermark=1;
	pe[21].precise_ip=1; /* constant skid */
	pe[21].mmap_data=1;
	pe[21].sample_id_all=1;
	pe[21].exclude_host=1;
	pe[21].exclude_callchain_user=1;
	pe[21].wakeup_watermark=0;
	pe[21].bp_type=HW_BREAKPOINT_EMPTY;

	fd[21]=perf_event_open(&pe[21],
				0, /* current thread */
				-1, /* all cpus */
				-1, /* New Group Leader */
				(long)0xffff880c00000000ULL /*ffff880c00000000*/ );


/* 18 */
/* fd = 22 */

	memset(&pe[22],0,sizeof(struct perf_event_attr));
	pe[22].type=PERF_TYPE_SOFTWARE;
	pe[22].size=72;
	pe[22].config=PERF_COUNT_SW_TASK_CLOCK;
	pe[22].sample_type=0; /* 0 */
	pe[22].read_format=PERF_FORMAT_TOTAL_TIME_RUNNING; /* 2 */
	pe[22].disabled=1;
	pe[22].pinned=1;
	pe[22].mmap=1;
	pe[22].freq=1;
	pe[22].inherit_stat=1;
	pe[22].precise_ip=0; /* arbitrary skid */
	pe[22].mmap_data=1;
	pe[22].sample_id_all=1;
	pe[22].exclude_guest=1;
	pe[22].exclude_callchain_kernel=1;
	pe[22].exclude_callchain_user=1;
	pe[22].wakeup_events=0;
	pe[22].bp_type=HW_BREAKPOINT_EMPTY;

	fd[22]=perf_event_open(&pe[22],
				getpid(), /* current thread */
				-1, /* all cpus */
				-1, /* New Group Leader */
				0 /*0*/ );


/* 19 */
	close(fd[16]);
/* 20 */
	prctl(PR_TASK_PERF_EVENTS_ENABLE);
/* 21 */
	prctl(PR_TASK_PERF_EVENTS_DISABLE);
	/* Replayed 21 syscalls */
	return 0;
}
