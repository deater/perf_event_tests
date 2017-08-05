/* ioctl_set_filter.c  */
/* Tests the PERF_EVENT_IOC_SET_FILTER functionality */

/* by Vince Weaver   vincent.weaver@maine.edu */

/* This sets a ftrace filter.
	ftrace events are found under /sys/kernel/debug/tracing/events
	Valid ftrace filter strings are based on the parameters found in
	/sys/kernel/debug/tracing/events/ * / * /format

	A way to generate them using perf:
		perf record -e irq:irq_handler_entry --filter irq==28 \
			-e irq:softirq_entry --filter vec==6 -a -- sleep 5
*/

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

static int quiet;

#define MAX_FILTER 8192
char filter[MAX_FILTER];

#define MAX_SEARCH	4096

int main(int argc, char** argv) {

	int fd1,i;
	struct perf_event_attr pe1;
	int errors=0;

	int result;

	char test_string[]="Testing PERF_EVENT_IOC_SET_FILTER ioctl...";

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing PERF_EVENT_IOC_SET_FILTER ioctl.\n");
	}

	/****************************************************/
	/* Check if /sys/kernel/debug/tracing/events exists */
	/****************************************************/
//	result=access("/sys/kernel/debug/tracing/events",F_OK);

	/* Actually this is pointless, as it gives EACCESS */
	/* as a normal user even if the file exists */

	/************************************/
	/* Creating a tracepoint event      */
	/************************************/

	if (!quiet) {
		printf("Creating a tracepoint event\n");
	}

	memset(&pe1,0,sizeof(struct perf_event_attr));
	pe1.type=PERF_TYPE_TRACEPOINT;
	pe1.size=sizeof(struct perf_event_attr);

	/* Find a trace event that will let us add a particular filter */

	/* It should work with */
	/* writeback:writeback_start*/
	/* but we can't get the id of this directly without debugfs/tracefs */
	/* mounted (the id numbers change depending on machine/kernel)      */

	/* Valid filter */
	strcpy(filter,"nr_pages==2");

	if (!quiet) {
		printf("Trying to find an event that will allow filter %s\n",
			filter);
	}

	/* Usually there are fewer than 1000 trace events? */
	for(i=0;i<MAX_SEARCH;i++) {

		pe1.config=i;
		pe1.disabled=1;
		pe1.exclude_kernel=0;
		pe1.exclude_hv=0;
		arch_adjust_domain(&pe1,quiet);

		fd1=perf_event_open(&pe1,0,-1,-1,0);
		if (fd1<0) {
			if (!quiet) {
				// fprintf(stderr,"Failed on %d\n",i);
			}
			continue;
		}

		result=ioctl(fd1, PERF_EVENT_IOC_SET_FILTER, filter);
		if (result==0) {
			if (!quiet) printf("Found proper event %d\n",i);
			close(fd1);
			break;
		}
		else {

		}
		close(fd1);
	}

	if (i==MAX_SEARCH) {
		if (!quiet) {
			printf("Could not find any trace event to filter\n");
		}
		test_skip(test_string);
		errors++;
	}

	pe1.config=i;
	pe1.disabled=1;
	pe1.exclude_kernel=0;
	pe1.exclude_hv=0;
	arch_adjust_domain(&pe1,quiet);

	/* Create group leader */
	fd1=perf_event_open(&pe1,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Unexpected error %s\n",strerror(errno));
		}
		test_fail(test_string);
	}

	for(i=0;i<MAX_FILTER;i++) {
		filter[i]=0xff;
	}

	/* Check a too big value */
	if (!quiet) {
		printf("\t+ Checking a too-big event: ");
	}
	result=ioctl(fd1, PERF_EVENT_IOC_SET_FILTER, filter);
	if ((result==-1) && (errno==EINVAL)) {
		if (!quiet) printf("Failed as expected\n");
	}
	else {
		if (!quiet) printf("Unexpected %d %s\n",result,strerror(errno));
		errors++;
	}

	/* Check off-by-one value */
	/* Size limited to pagesize */
	if (!quiet) {
		printf("\t+ Checking off-by-one filter: ");
	}
	filter[4096]=0;
	result=ioctl(fd1, PERF_EVENT_IOC_SET_FILTER, filter);
	if ((result==-1) && (errno==EINVAL)) {
		if (!quiet) printf("Failed as expected\n");
	}
	else {
		if (!quiet) printf("Unexpected %d %s\n",result,strerror(errno));
		errors++;
	}

	/* Check a just-right value */
	if (!quiet) {
		printf("\t+ Checking max size invalid filter: ");
	}
	filter[4095]=0;
	result=ioctl(fd1, PERF_EVENT_IOC_SET_FILTER, filter);
	if ((result==-1) && (errno==EINVAL)) {
		if (!quiet) printf("Failed as expected\n");
	}
	else {
		if (!quiet) printf("Unexpected %d %s\n",result,strerror(errno));
		errors++;
	}

	/* Check an empty value */
	if (!quiet) {
		printf("\t+ Checking empty filter: ");
	}
	filter[0]=0;
	result=ioctl(fd1, PERF_EVENT_IOC_SET_FILTER, filter);
	if ((result==-1) && (errno==EINVAL)) {
		if (!quiet) printf("Failed as expected\n");
	}
	else {
		if (!quiet) printf("Unexpected %d %s\n",result,strerror(errno));
		errors++;
	}

	/* Clear a filter */
	if (!quiet) {
		printf("\t+ Clear filter (write 0): ");
	}
	filter[0]='0';
	filter[1]=0;
	result=ioctl(fd1, PERF_EVENT_IOC_SET_FILTER, filter);
	if ((result==-1) && (errno==EINVAL)) {
		if (!quiet) printf("Failed as expected\n");
	}
	else {
		if (!quiet) printf("Unexpected %d %s\n",result,strerror(errno));
		errors++;
	}




	/* tracefs usually under /sys/kernel/tracing */



	/* start */
	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);

	/* million */
	result=instructions_million();

	/* stop */
	ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);

	close(fd1);

	if (errors) {
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}

