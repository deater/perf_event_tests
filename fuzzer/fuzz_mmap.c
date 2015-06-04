#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>

#include <signal.h>

#include "../include/perf_event.h"
#include "../include/perf_helpers.h"


#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_stats.h"

#define MAX_MMAPS	1024

struct our_mmap_t {
	int active;
	char *addr;
	int size;
	int fd;
	long long head;
} mmaps[MAX_MMAPS];

static int active_mmaps=0;

static int find_empty_mmap(void) {

	int i;

	for(i=0;i<MAX_MMAPS;i++) {
		if (!mmaps[i].active) {
			return i;
		}
	}
	return -1;

}

static int find_random_active_mmap(void) {

        int i,x,j=0;

        if (active_mmaps<1) {
//		printf("No active mmaps %d\n",active_mmaps);
		return -1;
	}

        x=rand()%active_mmaps;

        for(i=0;i<MAX_MMAPS;i++) {
                if (mmaps[i].active) {
                        if (j==x) return i;
                        j++;
                }
        }

	printf("Fail random mmaps\n");
        return -1;
}


long long perf_mmap_read(int which) {

	struct perf_event_mmap_page *control_page;

	long long head;

	stats.mmap_read_attempts++;

	control_page=(struct perf_event_mmap_page *)mmaps[which].addr;

	if (!mmaps[which].active) return 0;

	if (!mmaps[which].addr) return 0;

	if (mmaps[which].size==0) return 0;

	if (control_page==NULL) {
		return -1;
	}
	head=control_page->data_head;
	rmb(); /* Must always follow read of data_head */

	/* Mark all as read */
	control_page->data_tail=head;

	mmaps[which].head=head;

	stats.mmap_read_successful++;

	return head;
}


/* The first mmap() page is writeable so you can set the tail pointer */
/* So try over-writing it to see what happens.                        */

void trash_random_mmap(void) {

	int i,value;

	stats.mmap_trash_attempts++;

	i=find_random_active_mmap();

	/* Exit if no events */
	if (i<0) return;

	value=rand();

	if (ignore_but_dont_skip.trash_mmap) return;

	/* can't write high pages? */
	//event_data[i].mmap_size);

//	*(event_data[i].mmap)=0xff;

	memset(mmaps[i].addr,value, 1);//getpagesize());

	if (logging&TYPE_TRASH_MMAP) {
		sprintf(log_buffer,"Q %d %d %d\n",
			value,
			getpagesize(),
			mmaps[i].fd);
		write(log_fd,log_buffer,strlen(log_buffer));
	}


	stats.mmap_trash_successful++;

}

int setup_mmap(int which) {

	int i;

	i=find_empty_mmap();
	if (i<0) return -1;

	/* need locking? */
	mmaps[i].active=1;

	/* to be valid we really want to be 1+2^x pages */
	switch(rand()%3) {
		case 0: mmaps[i].size=(rand()%64)*getpagesize();
			break;
		case 1: mmaps[i].size=
				(1 + (1<<rand()%10) )*getpagesize();
			break;
		default: mmaps[i].size=rand()%65535;
	}

	mmaps[i].addr=NULL;

	if (!ignore_but_dont_skip.mmap) {

		stats.mmap_attempts++;
		mmaps[i].addr=mmap(NULL, mmaps[i].size,
			PROT_READ|PROT_WRITE, MAP_SHARED, event_data[which].fd, 0);

		if (mmaps[i].addr==MAP_FAILED) {
			mmaps[i].addr=NULL;
			mmaps[i].active=0;
#if LOG_FAILURES
			if (logging&TYPE_MMAP) {
				if (trigger_failure_logging) {
					sprintf(log_buffer,"# M %d %d %p\n",
						mmaps[i].size,
						event_data[which].fd,
						mmaps[i].addr);
					write(log_fd,log_buffer,
						strlen(log_buffer));
				}
			}
#endif
		}

		else {
			active_mmaps++;
			event_data[which].mmap=i;
			mmaps[i].fd=event_data[which].fd;

			if (logging&TYPE_MMAP) {
				sprintf(log_buffer,"M %d %d %p\n",
					mmaps[i].size,
					event_data[which].fd,
					mmaps[i].addr);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			stats.mmap_successful++;
		}
	}
	return 0;
}

void unmap_mmap(int i,int from_sigio) {

	int result;

	stats.mmap_unmap_attempts++;

	if (mmaps[i].addr==NULL) return;
//	printf("Unmapping %p\n",mmaps[i].addr);

	/* moved up to avoid a race with signal/read_mmap */
	mmaps[i].active=0;

	result=munmap(mmaps[i].addr,mmaps[i].size);
	if ((!from_sigio) && (logging&TYPE_MMAP)) {
		sprintf(log_buffer,"U %d %d %p\n",
			event_data[i].fd,
			mmaps[i].size,
			mmaps[i].addr);
		write(log_fd,log_buffer,strlen(log_buffer));
	}
	mmaps[i].addr=0;

	if (result==0) {
		stats.mmap_unmap_successful++;
		active_mmaps--;
	}
}


void mmap_random_event(int type) {

	int which;

	switch(rand()%6) {

		case 0:	/* mmap random */
			which=find_random_active_event();
			setup_mmap(which);
			break;
		case 1: /* aux random */
			break;
		case 2: /* munmap random */
			which=find_random_active_event();
			unmap_mmap(which,0);
			break;
		case 3: /* mmap read */
			which=find_random_active_event();
			perf_mmap_read(which);
			break;
		case 4: /* trash mmap */
			if (type & TYPE_TRASH_MMAP) {
				trash_random_mmap();
			}
			break;

		default:
			break;
	}

	return;
}
