#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/wait.h>

#include <signal.h>

#include "perf_event.h"

#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_stats.h"

#include "fuzz_close.h"
#include "fuzz_mmap.h"
#include "fuzz_overflow.h"
#include "fuzz_fork.h"

#define MAX_THROTTLES		10

int throttle_close_event=0;
struct sigaction sigio;
int next_overflow_refresh=0;
static int next_refresh=0;

/* Try to exit without leaving any children around */
void orderly_shutdown(void) {

	int status;

	if (already_forked) {
		printf("Trying to cleanly shudtdown pid %d\n",forked_pid);
		kill(forked_pid,SIGKILL);
		waitpid(forked_pid, &status, 0);
		printf("Done...\n");
	}

	printf("Trying to shut ourselves down: %d, last child %d\n",
			getpid(),forked_pid);

	exit(1);
}

//static long long prev_head=0;

/* The perf tool uses poll() and never sets signals */
/* Thus they never have most of these problems      */
void our_handler(int signum, siginfo_t *info, void *uc) {

	static int already_handling=0;

	int fd = info->si_fd;
	int i;
	int ret;
	static int last_fd=0;


	/* In some cases (syscall tracepoint) */
	/* The act of disabling an event would trigger */
	/* Another overflow, leading to a recursive storm */
	if (already_handling) {
		stats.already_overflows++;
		return;
	}

	already_handling=1;

	stats.overflows++;

	/* disable the event for the time being */
	/* we were having trouble with signal storms */
	ret=ioctl(fd,PERF_EVENT_IOC_DISABLE,0);
	/* Do not log, logging only make sense if */
	/* we have deterministic counts which we don't */

	/* Somehow we got a signal from an invalid event? */
	/* How would this happen?			*/
	/* Looks like if we fork() then close an event, */
	/* It can still be alive in the child and cause */
	/* a signal to come in even though it is closed.*/
	if (ret<0) {
		if (fd!=last_fd) {
			printf("Signal from invalid fd %d %s\n",
				fd,strerror(errno));
			last_fd=fd;
		}
		already_handling=0;
		return;
//		orderly_shutdown();
	}

	i=lookup_event(fd);

	if (i>=0) {

		event_data[i].overflows++;

		if (event_data[i].overflows>10000) {
			if (!logging) printf("Throttling event %d fd %d, last_refresh=%d, "
				"period=%llu, type=%d throttles %d\n",
				i,event_data[i].fd,event_data[i].last_refresh,
				event_data[i].attr.sample_period,
				event_data[i].attr.type,
				event_data[i].throttles);
			event_data[i].overflows=0;
			event_data[i].throttles++;

			/* otherwise if we re-trigger next time */
			/* with >1 refresh the throttle never   */
			/* lasts a significant amount of time.  */
			next_refresh=0;

			/* Avoid infinite throttle storms */
			if (event_data[i].throttles > MAX_THROTTLES) {

				printf("Stuck in a signal storm w/o forward progress; Max throttle count hit, giving up\n");
				close(event_data[i].fd);
//				orderly_shutdown();

				/* In a storm we used to try to somehow stop */
				/* it by closing all events, but this never  */
				/* really worked.			     */
#if 0
				/* Disable all events */
				printf("Trying to disable all events\n");
				for(j=0;j<NUM_EVENTS;j++) {
					if (event_data[j].active) {
						ioctl(event_data[j].fd,PERF_EVENT_IOC_DISABLE,0);
					}
				}

				throttle_close_event=i;
#endif
			}
		}

		else {

			/* read the event */
			perf_mmap_read(event_data[i].mmap);

			/* cannot call rand() from signal handler! */
			/* we re-enter and get stuck in a futex :( */

			ret=ioctl(fd, PERF_EVENT_IOC_REFRESH,next_refresh);
			if (ret==0) {
				event_data[i].last_refresh=next_refresh;
			}
			/* Do not log, makes no sense */
		}
	}

	already_handling=0;

	(void) ret;

}

void sigio_handler(int signum, siginfo_t *info, void *uc) {

	int fd = info->si_fd;
	long band = info->si_band;
	int code = info->si_code;
	int i;

	if (code & SI_KERNEL) {
		/* We overflowed the RT signal queue and the kernel	*/
		/* has rudely let us know w/o telling us which event	*/
		/* Close a random event in hopes of stopping		*/
		if (!logging) printf("SIGIO due to RT queue overflow\n");

		/* Close all events? */
		/* Extreme, but easier to re-play */
		/* Does make for non-deterministic traces :( */

		for(i=0;i<NUM_EVENTS;i++) {
			if (event_data[i].active) close_event(i,1);
		}
	}
	else {
		printf("SIGIO from fd %d band %lx code %d\n",fd,band,code);
	}

	stats.sigios++;
}
