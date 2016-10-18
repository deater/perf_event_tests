/* eoverflow.c */
/* Tests to see if EOVERFLOW properly returned by perf_event_open() */

/* This was introduced in Linux 4.8 */
/* 97c79a38cd454602645f0470ffb444b3b75ce574 */

/* By Vince Weaver vincent.weaver _at_ maine.edu */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include <errno.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"

int main(int argc, char **argv) {

	int fd;
	int quiet;
	int max_stack=1,curr_stack=1,failures=0;
	struct perf_event_attr attr;
	FILE *fff;

	char test_string[]="Testing EOVERFLOW generation...";

	quiet=test_quiet();

	if (!quiet) {
		printf("This tries to get perf_event_open() to generate\n");
		printf("an EOVERFLOW errno.\n\n");
	}

	fff=fopen("/proc/sys/kernel/perf_event_max_stack","r");
	if (fff==NULL) {
		test_skip(test_string);
	}
	fscanf(fff,"%d",&max_stack);
	fclose(fff);

	/****************/
	/* curr_stack=1 */
	/****************/

	if (!quiet) {
		printf("Trying with stack=%d max_stack=%d\n",
			curr_stack,max_stack);
	}

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_SOFTWARE;
	attr.size=sizeof(struct perf_event_attr);
	attr.config=PERF_COUNT_SW_CONTEXT_SWITCHES;
	attr.sample_period=100000;
	attr.sample_type=PERF_SAMPLE_CALLCHAIN;
	attr.sample_max_stack=curr_stack;

	attr.disabled=1;
	attr.pinned=1;
	attr.exclude_kernel=1;
	attr.exclude_hv=1;
	attr.wakeup_events=1;

	fd=perf_event_open(&attr,
				0, /* current thread */
				-1, /* any cpu */
				-1, /* New Group Leader */
				0 /*0*/ );

	if (fd<0) {
		if (!quiet) printf("\tUnexpectedly failed %s\n",
			strerror(errno));
		failures++;
	}
	else {
		if (!quiet) printf("\tsuccess!\n");
	}


	/******************/
	/* curr_stack=max */
	/******************/

	curr_stack=max_stack;

	if (!quiet) {
		printf("Trying with stack=%d max_stack=%d\n",
			curr_stack,max_stack);
	}

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_SOFTWARE;
	attr.size=sizeof(struct perf_event_attr);
	attr.config=PERF_COUNT_SW_CONTEXT_SWITCHES;
	attr.sample_period=100000;
	attr.sample_type=PERF_SAMPLE_CALLCHAIN;
	attr.sample_max_stack=curr_stack;

	attr.disabled=1;
	attr.pinned=1;
	attr.exclude_kernel=1;
	attr.exclude_hv=1;
	attr.wakeup_events=1;

	fd=perf_event_open(&attr,
				0, /* current thread */
				-1, /* any cpu */
				-1, /* New Group Leader */
				0 /*0*/ );

	if (fd<0) {
		if (!quiet) printf("\tUnexpectedly failed %s\n",
			strerror(errno));
		failures++;
	}
	else {
		if (!quiet) printf("\tsuccess!\n");
	}


	/********************/
	/* curr_stack=max+1 */
	/********************/

	curr_stack=max_stack+1;

	if (!quiet) {
		printf("Trying with stack=%d max_stack=%d\n",
			curr_stack,max_stack);
	}

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_SOFTWARE;
	attr.size=sizeof(struct perf_event_attr);
	attr.config=PERF_COUNT_SW_CONTEXT_SWITCHES;
	attr.sample_period=100000;
	attr.sample_type=PERF_SAMPLE_CALLCHAIN;
	attr.sample_max_stack=curr_stack;

	attr.disabled=1;
	attr.pinned=1;
	attr.exclude_kernel=1;
	attr.exclude_hv=1;
	attr.wakeup_events=1;

	fd=perf_event_open(&attr,
				0, /* current thread */
				-1, /* any cpu */
				-1, /* New Group Leader */
				0 /*0*/ );


	if (fd<0) {
		if (!quiet) printf("\tfailed %s\n",
			strerror(errno));
		if (errno!=EOVERFLOW) {
			if (!quiet) printf("Unexpected error return\n");
			failures++;
		}
	}
	else {
		if (!quiet) printf("\tUnexpectedly pass!\n");
		failures++;
	}

	/********************/
	/* curr_stack=huge  */
	/********************/

	curr_stack=65535;

	if (!quiet) {
		printf("Trying with stack=%d max_stack=%d\n",
			curr_stack,max_stack);
	}

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_SOFTWARE;
	attr.size=sizeof(struct perf_event_attr);
	attr.config=PERF_COUNT_SW_CONTEXT_SWITCHES;
	attr.sample_period=100000;
	attr.sample_type=PERF_SAMPLE_CALLCHAIN;
	attr.sample_max_stack=curr_stack;

	attr.disabled=1;
	attr.pinned=1;
	attr.exclude_kernel=1;
	attr.exclude_hv=1;
	attr.wakeup_events=1;

	fd=perf_event_open(&attr,
				0, /* current thread */
				-1, /* any cpu */
				-1, /* New Group Leader */
				0 /*0*/ );

	if (fd<0) {
		if (!quiet) printf("\tfailed %s\n",
			strerror(errno));
		if (errno!=EOVERFLOW) {
			if (!quiet) printf("Unexpected error return\n");
			failures++;
		}
	}
	else {
		if (!quiet) printf("\tUnexpectedly pass!\n");
		failures++;
	}

	/********************/
	/* curr_stack>16bit */
	/********************/

	/* Overflow, it's only a 16-bit field */
	curr_stack=65536;

	if (!quiet) {
		printf("Trying with stack=%d max_stack=%d\n",
			curr_stack,max_stack);
	}

	memset(&attr,0,sizeof(struct perf_event_attr));
	attr.type=PERF_TYPE_SOFTWARE;
	attr.size=sizeof(struct perf_event_attr);
	attr.config=PERF_COUNT_SW_CONTEXT_SWITCHES;
	attr.sample_period=100000;
	attr.sample_type=PERF_SAMPLE_CALLCHAIN;
	attr.sample_max_stack=curr_stack;

	attr.disabled=1;
	attr.pinned=1;
	attr.exclude_kernel=1;
	attr.exclude_hv=1;
	attr.wakeup_events=1;

	fd=perf_event_open(&attr,
				0, /* current thread */
				-1, /* any cpu */
				-1, /* New Group Leader */
				0 /*0*/ );

	if (fd<0) {
		if (!quiet) printf("\tfailed %s\n",
			strerror(errno));
		failures++;
	}
	else {
		if (!quiet) printf("\tpass due to wraparound!\n");
	}

	if (failures) test_fail(test_string);


	test_pass(test_string);

	return 0;
}
