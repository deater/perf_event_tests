#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <signal.h>

#include <sys/wait.h>

#include "instructions_testcode.h"
#include "perf_event.h"

#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_stats.h"

#include "fuzz_fork.h"

int already_forked=0;
pid_t forked_pid;

void fork_random_event(void) {

	int status;

	if (ignore_but_dont_skip.fork) return;

	if (already_forked) {

		if (logging&TYPE_FORK) {
			sprintf(log_buffer,"F 0\n");
			write(log_fd,log_buffer,strlen(log_buffer));
		}

		kill(forked_pid,SIGKILL);
		stats.total_syscalls++;

		/* not sure if this will cause us to miss bugs */
		/* but it does make the logs more deterministic */
		/* Update -- this will leak zombies unless we */
		/* always wait for the children.	*/
//		if (attempt_determinism) {
			waitpid(forked_pid, &status, 0);
			stats.total_syscalls++;
//		}

		already_forked=0;
	}
	else {

		if (logging&TYPE_FORK) {
			sprintf(log_buffer,"F 1\n");
			write(log_fd,log_buffer,strlen(log_buffer));
		}

		forked_pid=fork();

		/* we're the child */
		if (forked_pid==0) {
			while(1) {
				instructions_million();
				/* we were orphaned, exit */
				/* Had problems with orphans clogging up */
				/* the system if the parent emergency */
				/* exited */
				if (getppid()==1) {
					exit(1);
				}
			}
		}

		stats.total_syscalls++;
		stats.fork_attempts++;

		/* We do see failures sometimes */
		/* And when we do, if we foolishly kill process "-1" */
		/* It will kill *all* processes beloning to the user */
		/* Logging you out on all windows.                   */
		if (forked_pid==-1) {
//			printf("Fork failed! %s\n",strerror(errno));
			if (errno<MAX_ERRNOS) {
				stats.fork_errno_count[errno]++;
			}
			already_forked=0;
		}
		else {
			stats.fork_successful++;
			already_forked=1;
		}
	}
}
