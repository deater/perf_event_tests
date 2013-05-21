#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "shm.h"
#include "syscall.h"
#include "../include/perf_event.h"
#include "../include/perf_helpers.h"
#include "../include/instructions_testcode.h"

int user_set_seed;
int page_size=4096;

struct shm_s *shm;


char *page_rand;

#define NUM_EVENTS 1024

struct event_data_t{
	int active;
	int fd;
	struct perf_event_attr attr;
	pid_t pid;
	int cpu;
	int group_fd;
	unsigned long flags;
} event_data[NUM_EVENTS];

extern struct syscall syscall_perf_event_open;

static int active_events=0;

static int find_random_event(void) {

	int i,x,j=0;

	if (active_events<1) return -1;

	x=rand()%active_events;

	for(i=0;i<NUM_EVENTS;i++) {
		if (event_data[i].active) {
			if (j==x) break;
			j++;
		}
	}
	return i;
}

static int find_empty_event(void) {

	int i;

	for(i=0;i<NUM_EVENTS;i++) {
		if (!event_data[i].active) {
			active_events++;
			return i;
		}
	}
	return -1;

}

static void open_random_event(void) {

	int fd;

	int i;

	i=find_empty_event();

	/* return if no free events */
	if (i<0) return;

	while(1) {
		syscall_perf_event_open.sanitise(0);
		memcpy(&event_data[i].attr,
			(struct perf_event_attr *)shm->a1[0],
			sizeof(struct perf_event_attr));
		event_data[i].pid=shm->a2[0];
		event_data[i].cpu=shm->a3[0];
		event_data[i].group_fd=shm->a4[0];
		event_data[i].flags=shm->a5[0];

#if 0
		printf("Trying pid=%d cpu=%d group_fd=%d flags=%lx\n",
			event_data[i].pid,
			event_data[i].cpu,
			event_data[i].group_fd,
			event_data[i].flags);
#endif
		fd=perf_event_open(&event_data[i].attr,
			event_data[i].pid,
			event_data[i].cpu,
			event_data[i].group_fd,
			event_data[i].flags);

		if (fd>0) break;

	}
	printf("Opening fd=%d Active=%d\n",fd,active_events);
	event_data[i].fd=fd;
	event_data[i].active=1;
}

static void close_random_event(void) {

	int i;

	i=find_random_event();

	/* Exit if no events */
	if (i<0) return;

	active_events--;
	printf("Closing %d, Active=%d\n",
		event_data[i].fd,active_events);
	close(event_data[i].fd);
	event_data[i].active=0;
}

static void ioctl_random_event(void) {


	int i,arg,arg2;

	i=find_random_event();

	/* Exit if no events */
	if (i<0) return;

	switch(rand()%8) {
		case 0:
			printf("ENABLE fd %d\n",event_data[i].fd);
			ioctl(event_data[i].fd,PERF_EVENT_IOC_ENABLE,0);
			break;
		case 1:
			printf("DISABLE fd %d\n",event_data[i].fd);
			ioctl(event_data[i].fd,PERF_EVENT_IOC_DISABLE,0);
			break;
		case 2: arg=rand();
			printf("REFRESH fd %d %d\n",event_data[i].fd,arg);
			ioctl(event_data[i].fd,PERF_EVENT_IOC_REFRESH,arg);
			break;
		case 3:
			printf("RESET fd %d\n",event_data[i].fd);
			ioctl(event_data[i].fd,PERF_EVENT_IOC_RESET,0);
			break;
		case 4: arg=rand();
			printf("PERIOD fd %d %d\n",event_data[i].fd,arg);
			ioctl(event_data[i].fd,PERF_EVENT_IOC_PERIOD,arg);
			break;
		case 5: arg=rand();
			printf("SET_OUTPUT fd %d %d\n",event_data[i].fd,arg);
			ioctl(event_data[i].fd,PERF_EVENT_IOC_SET_OUTPUT,arg);
			break;
		case 6: arg=rand();
			printf("SET_FILTER fd %d %d\n",event_data[i].fd,arg);
			ioctl(event_data[i].fd,PERF_EVENT_IOC_SET_FILTER,arg);
			break;
		default:
			arg=rand(); arg2=rand();
			ioctl(event_data[i].fd,arg,arg2);
			printf("RANDOM_IOCTL fd %d %x %x\n",
				event_data[i].fd,arg,arg2);
			break;
	}
}

static void prctl_random_event(void) {

}

static void read_random_event(void) {

}

static void write_random_event(void) {

}

static void access_random_file(void) {

}

static void run_a_million_instructions(void) {

	instructions_million();

}


int main(int argc, char **argv) {

	int i;

	/* Set up to match trinity setup, vaguely */

	shm=calloc(1,sizeof(struct shm_s));

	page_rand = memalign(page_size, page_size * 2);
	if (!page_rand) {
		exit(EXIT_FAILURE);
	}
	memset(page_rand, 0x55, page_size);
	fprintf(stderr, "page_rand @ %p\n", page_rand);

	/* Initialize */
	for(i=0;i<NUM_EVENTS;i++) {
		event_data[i].active=0;
	}

	while(1) {

		switch(rand()%8) {
			case 0:	open_random_event();
				break;
			case 1: close_random_event();
				break;
			case 2: ioctl_random_event();
				break;
			case 3: prctl_random_event();
				break;
			case 4: read_random_event();
				break;
			case 5: write_random_event();
				break;
			case 6: access_random_file();
				break;
#if 0
			case 7: fork_random_event();
				break;
#endif
			default:
				run_a_million_instructions();
				break;
		}
	}

	return 0;

}
