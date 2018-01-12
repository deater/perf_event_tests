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

#define MAX_READ_SIZE 65536

static long long data[MAX_READ_SIZE];

void read_random_event(void) {

	int i,result,read_size;

	i=find_random_active_event();

	/* Exit if no events */
	if (i<0) return;

	/* Exit if event has fd of 0, not want to read stdin */
	if (event_data[i].fd==0) return;

	switch (rand()%4) {
		case 0:	read_size=event_data[i].read_size;
			break;
		case 1: read_size=(rand()%8)*sizeof(long long);
			break;
		case 2: read_size=(rand()%MAX_READ_SIZE);
			break;
		default: read_size=(rand()%MAX_READ_SIZE)*sizeof(long long);
	}

	stats.read_attempts++;
	stats.total_syscalls++;

	if (ignore_but_dont_skip.read) return;
	result=read(event_data[i].fd,data,read_size);

	if (result>0) {
	        stats.read_successful++;
		if (logging&TYPE_READ) {
			sprintf(log_buffer,"R %d %d\n",
				event_data[i].fd,read_size);
			write(log_fd,log_buffer,strlen(log_buffer));
		}

	}

}
