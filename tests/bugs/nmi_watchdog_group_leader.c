/* nmi_watchdog_group_leader  --- by Vince Weaver vincent.weaver@maine.edu */

/* If the NMI watchdog is enabled it will steal a performance counter.   */
/* There is a bug that if you try to use the maximum number of counters  */
/*   (not counting the stolen one) with a group leader, sys_perf_open()  */
/* will indicate success, as will starting the count, but you will fail  */
/* at read time.                                                         */

/* This bug still exists and will probably never be fixed	*/
/* The perf NMI watchdog was not introduced until 2.6.34	*/

/* This also triggers in the case of the schedulability bug	*/
/* but since that was fixed in 2.6.34 then in theory there is	*/
/* no overlap in the tests.					*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <unistd.h>
#include <asm/unistd.h>

#include <sys/ioctl.h>
#include <errno.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"


int main(int argc, char **argv) {

	int events[MAX_TEST_EVENTS];
	int fd[MAX_TEST_EVENTS];
	int count=MAX_TEST_EVENTS;

	int i,j,n;
	int ret1,ret2,ret3;

	struct perf_event_attr pe;

	int quiet,nmi_enabled;
	char test_string[]="Testing if NMI watchdog interferes w/ event scheduling...";

	quiet=test_quiet();

	if (!quiet) {
		printf("If the NMI watchdog is enabled it will steal a performance\n");
		printf("   counter.  There is a bug that if you try to use the\n");
		printf("   maximum number of counters anyway, sys_perf_open()\n");
		printf("   will succede but will fail when you try to read()\n");
		printf("   the values.\n\n");
	}

	nmi_enabled=detect_nmi_watchdog();
	if (!nmi_enabled) {
		if (!quiet) {
			printf("No NMI watchdog detected, so skipping test.\n\n");
		}
		test_skip(test_string);
	}

	if (copy_events(events)) {
		test_unimplemented(test_string);
	}

	for(i=1;i<count;i++) {

		if (!quiet) printf("Trying %d events:\n",i);

		memset(&pe,0,sizeof(struct perf_event_attr));

		pe.type=PERF_TYPE_RAW;
		pe.size=sizeof(struct perf_event_attr);
		pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;

		fd[0]=-1;

		for(j=0;j<i;j++) {
			pe.config=events[j];

			if (j==0) {
				pe.disabled=1;
				pe.pinned=1;
			}
			else {
				pe.disabled=0;
				pe.pinned=0;
			}

			fd[j]=perf_event_open(&pe,0,-1,fd[0],0);
			if (fd[j]<0) {
				if (!quiet) printf("Finished after event %d\n\n",j);
				test_pass(test_string);
				exit(1);
			}
		}

		/* start */
		ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

		/* stop */
		ret2=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

#define BUFFER_SIZE 256
		long long buffer[BUFFER_SIZE];

		ret3=read(fd[0],buffer,BUFFER_SIZE*sizeof(long long));

		if (ret3<=0) {
			if (!quiet) {
				printf("Unexpected read result %d %s\n",ret3,strerror(errno));
			}
			/* This has been known forever */
			/* Looks like it will never get fixed */
			test_known_kernel_bug(test_string);
		}

		if (buffer[0]!=j){
			if (!quiet) printf("Error!  buffer count is %lld not %d!\n",
				buffer[0],j);
			test_fail(test_string);
		}

		if (ret1<0) {
			if (!quiet) printf("Error starting!\n");
			test_fail(test_string);
		}

		if (ret2<0) {
			if (!quiet) printf("Error stopping!\n");
			test_fail(test_string);
		}

		if (!quiet) {
			for(n=0;n<buffer[0];n++) {
				printf("\t%x %lld\n",events[n],buffer[1+(n*2)]);
			}
		}

		for(n=0;n<j;n++) {
			close(fd[n]);
		}
	}

	test_pass(test_string);

	return 0;
}
