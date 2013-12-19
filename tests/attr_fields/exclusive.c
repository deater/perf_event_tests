/* exclusive.c  */

/* Test if the exclusive bit works                                            */

/*  The exclusive bit says to only run if *no* other events are running       */

/*  This fails on most distros as the NMI watchdog timer tends to be          */
/*  enabled and it uses an always-counting performance counter.               */

/*  Other counter use can also block exclusive events; for example            */
/*  another user doing a global measurement with "perf stat -a /bin/sleep 100 */
/*  will also lead to an exclusive event never getting scheduled (and thus    */
/*  always return 0 as a result.                                              */

/* by Vince Weaver   vincent.weaver@maine.edu */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include <signal.h>

#include <sys/mman.h>

#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/prctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

int main(int argc, char** argv) {

	int ret,fd1,quiet,i;
	int result;
	int nmi_enabled;

	struct perf_event_attr pe;

	char test_string[]="Testing exclusive eventsets...";

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing exclusive eventsets.\n");
	}

	nmi_enabled=detect_nmi_watchdog();

	/* set up group leader */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.pinned=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	pe.exclusive=1;

	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);

	/* enable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);

	instructions_million();

	/* disable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	#define BUFFER_SIZE 1
	long long buffer[BUFFER_SIZE];
	for(i=0;i<BUFFER_SIZE;i++) {
		buffer[i]=-1;
	}

	result=read(fd1,buffer,BUFFER_SIZE*sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("NMI watchdog enabled: %d\n",nmi_enabled);
		printf("Value    [%d] : %lld\n",0,buffer[0]);
	}


	/* If nmi watchdog enabled, should fail */
	if (nmi_enabled) {
		if (buffer[0]!=0) {
			if (!quiet) {
				fprintf(stderr,"Unexpectedly able to run events\n");
			}
			test_fail(test_string);
		}
	}
	else {
		if (buffer[0]==0) {

			if (!quiet) {
				fprintf(stderr,"Eventset creation failed for unknown reason\n");
				fprintf(stderr,"Is the kernel using an event?  Is another user?\n");
			}
			test_fail(test_string);
		}

	}


	test_pass(test_string);

	return 0;
}

