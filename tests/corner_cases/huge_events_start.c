/* This test tries to open the max num events, then start them  */
/* This limit is set by the maximum number of file descriptors  */
/* Which can be read by */
/*    ulimit -Sn    -- soft limit (1024 on my machine) */
/*    ulimit -Hn    -- hard limit (4096 on my machine) */

/* This has been raised to 65536 on my Debian Jessie box -- 1Sep2015 */

/* by Vince Weaver, vincent.weaver _at_ maine.edu                  */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"

char test_string[]="Testing start of max events...";

/* Recent systems have the limit set really high             */
/* Stop at 10000 as things get drastically slower after that */
#define DEFAULT_MAX_LIMIT 10000

int main(int argc, char **argv) {

	int quiet,i;
	struct perf_event_attr pe;
	int *fd;
	int max_limit=DEFAULT_MAX_LIMIT;
	int max_events;

	if (argc>1) {
		max_limit=atoi(argv[1]);
		if (max_limit<=0) {
			printf("Improper max limit %d\n",max_limit);
		}
	}

	quiet=test_quiet();

	fd=calloc(max_limit,sizeof(int));
	if (fd==NULL) {
		printf("Error allocating %d fds\n",max_limit);
		test_fail(test_string);
	}

	for(i=0;i<max_limit;i++) {

		if (!quiet) {
//			if (i%100==0) printf("Trying event %d\n",i);
		}

		memset(&pe,0,sizeof(struct perf_event_attr));
		pe.type=PERF_TYPE_HARDWARE;
		pe.size=sizeof(struct perf_event_attr);
		pe.config=PERF_COUNT_HW_INSTRUCTIONS;
		pe.disabled=1;
		pe.exclude_kernel=1;
		pe.exclude_hv=1;

		fd[i]=perf_event_open(&pe,0,-1,-1,0);
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
		printf("Trying to start %d events!\n",i);
	}

	for(i=0;i<max_events;i++) {
		ioctl(fd[i], PERF_EVENT_IOC_RESET, 0);
		ioctl(fd[i], PERF_EVENT_IOC_ENABLE,0);
	}

	/* do something */

	for(i=0;i<max_events;i++) {
		ioctl(fd[i], PERF_EVENT_IOC_DISABLE,0);
	}

	test_pass( test_string );

	return 0;
}


