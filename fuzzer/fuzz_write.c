#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <signal.h>

#include "perf_event.h"

#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_stats.h"

#define MAX_WRITE_SIZE 65536

static long long data[MAX_WRITE_SIZE];

void write_random_event(void) {

	int i,result,write_size;

	i=find_random_active_event();

	/* Exit if no events */
	if (i<0) return;

	/* Exit if event has fd of 0, not want to read stdin */
	if (event_data[i].fd==0) return;

	switch (rand()%4) {
		case 0:	write_size=event_data[i].read_size;
			break;
		case 1: write_size=(rand()%8)*sizeof(long long);
			break;
		case 2: write_size=(rand()%MAX_WRITE_SIZE);
			break;
		default: write_size=(rand()%MAX_WRITE_SIZE)*sizeof(long long);
	}

	if (ignore_but_dont_skip.write) return;

	stats.write_attempts++;
	stats.total_syscalls++;

	result=write(event_data[i].fd,data,write_size);

	/* logging */
	if (result>0) {
	        stats.write_successful++;
		if (logging&TYPE_WRITE) {
			sprintf(log_buffer,"W %d %d\n",event_data[i].fd,
						write_size);
			write(log_fd,log_buffer,strlen(log_buffer));
		}
	}

}
