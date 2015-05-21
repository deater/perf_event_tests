#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <signal.h>

#include "../include/perf_event.h"
#include "../include/perf_helpers.h"


#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_stats.h"

long long perf_mmap_read( void *our_mmap,
				int mmap_size,
				long long prev_head) {

	struct perf_event_mmap_page *control_page = our_mmap;
	long long head;

	if (!our_mmap) return 0;

	if (mmap_size==0) return 0;

	if (control_page==NULL) {
		return -1;
	}
	head=control_page->data_head;
	rmb(); /* Must always follow read of data_head */

	/* Mark all as read */
	control_page->data_tail=head;

	return head;
}


/* The first mmap() page is writeable so you can set the tail pointer */
/* So try over-writing it to see what happens.                        */

void trash_random_mmap(void) {

	int i,value;

	i=find_random_active_event();

	/* Exit if no events */
	if (i<0) return;

	if ((event_data[i].mmap)) {// && (rand()%2==1)) {

		value=rand();

		if (ignore_but_dont_skip.trash_mmap) return;

		/* can't write high pages? */
		//event_data[i].mmap_size);

//		*(event_data[i].mmap)=0xff;

		memset(event_data[i].mmap,value, 1);//getpagesize());

		if (logging&TYPE_TRASH_MMAP) {
			sprintf(log_buffer,"Q %d %d %d\n",
				value,
				getpagesize(),
				event_data[i].fd);
			write(log_fd,log_buffer,strlen(log_buffer));
		}


	}

	stats.trash_mmap_attempts++;
	stats.trash_mmap_successful++;

}
