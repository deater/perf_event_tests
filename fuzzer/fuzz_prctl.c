#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/prctl.h>

#include <signal.h>

#include "perf_event.h"

#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_stats.h"

#include "fuzz_prctl.h"

void prctl_random_event(void) {

	int ret;
	int type;

	stats.prctl_attempts++;
	stats.total_syscalls++;

	type=rand()%2;

	if (ignore_but_dont_skip.prctl) return;

	if (type) {
		ret=prctl(PR_TASK_PERF_EVENTS_ENABLE);
		if ((ret==0)&&(logging&TYPE_PRCTL)) {
			sprintf(log_buffer,"P 1\n");
			write(log_fd,log_buffer,strlen(log_buffer));
		}
	}
	else {
		ret=prctl(PR_TASK_PERF_EVENTS_DISABLE);
		if ((ret==0)&&(logging&TYPE_PRCTL)) {
			sprintf(log_buffer,"P 0\n");
			write(log_fd,log_buffer,strlen(log_buffer));
		}

	}

	/* FIXME: are there others we should be trying? */

	if (ret==0) stats.prctl_successful++;
}
