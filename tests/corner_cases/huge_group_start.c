/* This test tries to open the max num events in a group, then reads them  */

/* This turned up a bug in the Linux kernel!
   The kernel internal read_size value is a u16 (16-bit)
   But we can easily overflow that if we have a large number of events
	in a group.
  In this case a read() syscall misses the sanity check for the requested
	buffer being too small for the read, and the kernel will happily
	overflow past the buffer size we requested.

  This bug was reported on 2 September 2015.

  While annoying, I am not aware of any easy to exploit security implications
	of this bug, but I am always impressed with how clever people can be */

/* Vince Weaver -- <vincent.weaver@maine.edu> */

/* This was fixed in a723968c0ed36db676478c3d26078f13484fe01c (Linux 4.3) */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"

char test_string[]="Testing start of max event group...";

/* Recent systems have the limit set really high             */
/* Stop at 10000 as things get drastically slower after that */
#define DEFAULT_MAX_LIMIT 10000

#define DATASIZE 8192

#define SENTINEL 0xfeb131978

int main(int argc, char **argv) {

	int quiet,i;
	struct perf_event_attr pe;
	int *fd;
	int max_limit=DEFAULT_MAX_LIMIT;
	unsigned long long data[DATASIZE+1];
	int result;
	int max_events;

	data[DATASIZE]=SENTINEL;

	quiet=test_quiet();

	if (!quiet) printf("data size=%zd\n",sizeof(data));

	if (argc>1) {
		max_limit=atoi(argv[1]);
		if (max_limit<=0) {
			printf("Improper max limit %d\n",max_limit);
		}
	}

	fd=calloc(max_limit,sizeof(int));
	if (fd==NULL) {
		printf("Error allocating %d fds\n",max_limit);
		test_fail(test_string);
	}

	fd[0]=-1;

	for(i=0;i<max_limit;i++) {

		if (!quiet) {
		//	if (i%100==0) printf("Trying event %d\n",i);
		}

		memset(&pe,0,sizeof(struct perf_event_attr));

		if (i==0) {
			pe.type=PERF_TYPE_HARDWARE;
			pe.config=PERF_COUNT_HW_INSTRUCTIONS;
			pe.disabled=1;
		}
		else {
			pe.disabled=0;
			pe.type=PERF_TYPE_SOFTWARE;
			switch(i%5) {
				case 0: pe.config=PERF_COUNT_SW_CPU_CLOCK;
					break;
				case 1: pe.config=PERF_COUNT_SW_TASK_CLOCK;
					break;
				case 2: pe.config=PERF_COUNT_SW_PAGE_FAULTS;
					break;
				case 3: pe.config=PERF_COUNT_SW_CONTEXT_SWITCHES;
					break;
				case 4: pe.config=PERF_COUNT_SW_CPU_MIGRATIONS;
					break;
			}
		}
		pe.size=sizeof(struct perf_event_attr);
		pe.exclude_kernel=1;
		pe.exclude_hv=1;
		pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID|PERF_FORMAT_TOTAL_TIME_RUNNING|PERF_FORMAT_TOTAL_TIME_ENABLED;

		fd[i]=perf_event_open(&pe,0,-1,fd[0],0);
		if (fd[i]<0) {
			if (i==0) {
				if (!quiet) {
					printf("could not open event\n");
				}
				test_skip(test_string);
			}

			if (errno==EMFILE) {
				if (!quiet) {
					printf("Ran out of file descriptors at %d %s\n",i,strerror(errno));
				}
				break;

			} else {
				if (!quiet) {
					printf("Unexpected error at %d %s\n",i,strerror(errno));
				}
				test_fail(test_string);
				exit(0);

			}
		}

	}
	max_events=i;

	if (!quiet) {
		printf("Trying to start %d events!\n",max_events);
	}

	ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	/* do something */

	ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	if (!quiet) {
		printf("Trying to read %zd bytes into data\n",
			DATASIZE*sizeof(long long));
	}
	result=read(fd[0],&data,29*1024);// DATASIZE*sizeof(long long));

	if (data[DATASIZE]!=SENTINEL) {
		if (!quiet) {
			printf("Error! Kernel writes past end of buffer!\n");

		}

		/* This was fixed in Linux 4.3 */
		if (check_linux_version_newer(4,3,0)) {
			test_fail(test_string);
		}
		else {
			test_known_kernel_bug(test_string);
		}
	}

	if (!quiet) {
		if (result<0) {
			printf("Error!  %d %s\n",errno,strerror(errno));
		}
		printf("Read %d bytes\n",result);
	}

	for(i=0;i<max_events;i++) {
		close(fd[i]);
	}

	free(fd);


	test_pass( test_string );

	return 0;
}


