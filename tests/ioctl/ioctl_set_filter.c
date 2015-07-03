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

	/************************************/
	/* Creating a tracepoint event      */
	/************************************/

	if (!quiet) {
		printf("Creating a tracepoint event\n");
	}

	memset(&pe1,0,sizeof(struct perf_event_attr));
	pe1.type=PERF_TYPE_TRACEPOINT;
	pe1.size=sizeof(struct perf_event_attr);

	/* Try to pick one that is valid on most kernels/machines */
	/* writeback:writeback_start*/
	pe1.config=226;

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

	/* Valid filter */
	if (!quiet) {
		printf("\t+ Valid filter: ");
	}
	strcpy(filter,"nr_pages==2");

	result=ioctl(fd1, PERF_EVENT_IOC_SET_FILTER, filter);
	if (result==0) {
		if (!quiet) printf("Success\n");
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

