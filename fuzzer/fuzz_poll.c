#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <poll.h>

#include <signal.h>

#include "perf_event.h"

#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_stats.h"

#include "fuzz_poll.h"

#define MAX_POLL_FDS 128

void poll_random_event(void) {

	int i,result,num_fds;

	struct pollfd pollfds[MAX_POLL_FDS];
	int timeout;
	char log_buffer[BUFSIZ];

	num_fds=rand()%MAX_POLL_FDS;

	for(i=0;i<num_fds;i++) {
		pollfds[i].fd=event_data[find_random_active_event()].fd;
		pollfds[i].events=POLLIN;
	}

	/* Want short timeout (ms) */
	timeout=rand()%10;

	if (ignore_but_dont_skip.poll) return;

	stats.poll_attempts++;
	stats.total_syscalls++;
	result=poll(pollfds,num_fds,timeout);

	if (result>0) {
	        stats.poll_successful++;
		if (logging&TYPE_POLL) {
			sprintf(log_buffer,"p %d ",num_fds);
			write(log_fd,log_buffer,strlen(log_buffer));
			for(i=0;i<num_fds;i++) {
				sprintf(log_buffer,"%d %x ",pollfds[i].fd,
							pollfds[i].events);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			sprintf(log_buffer,"%d\n",timeout);
			write(log_fd,log_buffer,strlen(log_buffer));
		}
	}

}
