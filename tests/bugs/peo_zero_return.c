/* peo_zero_return */
/* Tests a bug in the 5.4 kernel where perf_event_open() returns 0 */
/* as a result, even if 0 is not an available fd */
/* Issue is a bug in kernel when aux_sample_size is invalid */

/* by Vince Weaver <vincent.weaver _at_ maine.edu> */

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
#include <hw_breakpoint.h>

#include "perf_event.h"
#include "test_utils.h"


static int fd;
static struct perf_event_attr pe;


FILE *fff;


int perf_event_open(struct perf_event_attr *hw_event_uptr,
	pid_t pid, int cpu, int group_fd, unsigned long flags) {

	return syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,
		group_fd, flags);
}

int main(int argc, char **argv) {

	char test_string[]="Testing if aux_sample_size triggers invalid fd...";
	int quiet;

        quiet=test_quiet();

	if (!quiet) {
		printf("Kernel 5.4 had a bug where aux_sample_size being set\n");
		printf("would cause perf_event_open() to return 0 even though\n");
		printf("the event creation failed.\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_RAW;
	pe.size=120;
	pe.config=0x0ULL;
	pe.sample_period=0x4777c3ULL;
	pe.sample_type=PERF_SAMPLE_STREAM_ID; /* 200 */
	pe.read_format=PERF_FORMAT_TOTAL_TIME_RUNNING|PERF_FORMAT_ID|PERF_FORMAT_GROUP; /* e */
	pe.inherit=1;
	pe.exclude_hv=1;
	pe.exclude_idle=1;
	pe.enable_on_exec=1;
	pe.watermark=1;
	pe.precise_ip=0; /* arbitrary skid */
	pe.mmap_data=1;
	pe.exclude_guest=1;
	pe.exclude_callchain_kernel=1;
	pe.mmap2=1;
	pe.comm_exec=1;
	pe.context_switch=1;
	pe.bpf_event=1;
	pe.wakeup_watermark=47545;
	pe.bp_type=HW_BREAKPOINT_EMPTY;
	pe.branch_sample_type=PERF_SAMPLE_BRANCH_KERNEL|PERF_SAMPLE_BRANCH_ANY_RETURN|PERF_SAMPLE_BRANCH_COND|0x800ULL;
	pe.sample_regs_user=42ULL;
	pe.sample_stack_user=0xfffffffd;
	pe.aux_watermark=25443;
	pe.aux_sample_size=8192;

	fd=perf_event_open(&pe,
				-1, /* current thread */
				0, /* Only cpu 0 */
				-1, /* New Group Leader */
				PERF_FLAG_FD_OUTPUT|PERF_FLAG_FD_CLOEXEC /*a*/ );

	if (fd==0) {
		if (!quiet) {
			fprintf(stderr,
				"Invalid %d return from perf_event_open()\n",
				fd);
		}

		if ( (check_linux_version_newer(5,4,0)) &&
			(check_linux_version_older(5,6,0))) {
			test_known_kernel_bug(test_string);
		}
		else {
			test_fail(test_string);
		}
	}

	test_pass(test_string);

	return 0;
}
