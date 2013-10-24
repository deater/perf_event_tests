/* log_to_code output from ./broken_trace/new.broken.7 */
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
#include <poll.h>
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>

int fd[1024];
struct perf_event_attr pe[1024];
char *mmap_result[1024];
#define MAX_READ_SIZE 65536
static long long data[MAX_READ_SIZE];

#define MAX_POLL_FDS 128
struct pollfd pollfds[MAX_POLL_FDS];

long long id;

int forked_pid;

struct sigaction sa;
static int overflows=0;
static int sigios=0;

static void our_handler(int signum, siginfo_t *info, void *uc) {
	int fd = info->si_fd;
	int ret;

	overflows++;
	ioctl(fd,PERF_EVENT_IOC_DISABLE,0);
	if (sigios) return;
	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH,1);
}
int perf_event_open(struct perf_event_attr *hw_event_uptr,
	pid_t pid, int cpu, int group_fd, unsigned long flags) {

	return syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,
		group_fd, flags);
}

int main(int argc, char **argv) {
/* 1 */

	memset(&pe[5],0,sizeof(struct perf_event_attr));
	pe[5].type=PERF_TYPE_TRACEPOINT;
	pe[5].size=64;
	pe[5].config=0x7fffffff00000001;
	pe[5].sample_period=0xffffffffff000000;

	pe[5].sample_type=PERF_SAMPLE_IP|PERF_SAMPLE_TIME|PERF_SAMPLE_READ|PERF_SAMPLE_ID|PERF_SAMPLE_PERIOD|PERF_SAMPLE_STREAM_ID|PERF_SAMPLE_WEIGHT; /* 4355 */
//	pe[5].read_format=PERF_FORMAT_ID; /* 4 */
//	pe[5].exclusive=1;
//	pe[5].exclude_user=1;
//	pe[5].exclude_idle=1;
//	pe[5].enable_on_exec=1;
//	pe[5].watermark=1;
//	pe[5].precise_ip=2; /* request zero skid */
//	pe[5].sample_id_all=1;
//	pe[5].wakeup_watermark=255;
//	pe[5].bp_type=HW_BREAKPOINT_EMPTY;

	fd[5]=perf_event_open(&pe[5],0,0,-1,0 /*0*/ );

/* 2 */
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;
	if (sigaction( SIGRTMIN+2, &sa, NULL) < 0) {
		printf("Error setting up signal handler\n");
	}
	fcntl(fd[5], F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd[5], F_SETSIG, SIGRTMIN+2);
	fcntl(fd[5], F_SETOWN,getpid());
	/* Replayed 2 syscalls */
	return 0;
}
