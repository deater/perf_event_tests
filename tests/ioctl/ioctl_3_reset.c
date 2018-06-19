/* ioctl_3_reset.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* Test PERF_EVENT_IOC_RESET  */

/* PERF_IOC_FLAG_GROUP was broken from  75f937f24bd9 (2.6.31) */
/*                                until 724b6daa1    (3.4)    */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>


#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"


#define EVENTS 2

#define READ_SIZE (EVENTS + 1)

static char test_string[]="Testing ioctl(PERF_IOC_RESET)...";

int main(int argc, char** argv) {

	int fd[EVENTS],ret,quiet;
	int result;
	int read_result;
	long long count[READ_SIZE];
	int i;
	int failures=0;

	struct perf_event_attr pe[EVENTS];

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing ioctl(PERF_IOC_RESET);\n");
	}

	/* Test if Reset of leader resets all */

	/* setup instruction event, group leader */
	memset(&pe[0],0,sizeof(struct perf_event_attr));

	pe[0].type=PERF_TYPE_HARDWARE;
	pe[0].size=sizeof(struct perf_event_attr);
	pe[0].config=PERF_COUNT_HW_INSTRUCTIONS;
	pe[0].disabled=1;
	pe[0].exclude_kernel=1;
	pe[0].read_format=PERF_FORMAT_GROUP;

	fd[0]=perf_event_open(&pe[0],0,-1,-1,0);
	if (fd[0]<0) {
		fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	/* setup cycles event, group child */
	memset(&pe[1],0,sizeof(struct perf_event_attr));

	pe[1].type=PERF_TYPE_HARDWARE;
	pe[1].size=sizeof(struct perf_event_attr);
	pe[1].config=PERF_COUNT_HW_CPU_CYCLES;
	pe[1].disabled=0;
	pe[1].exclude_kernel=1;

	fd[1]=perf_event_open(&pe[1],0,-1,fd[0],0);
	if (fd[1]<0) {
		fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	/* Count a million */
	ioctl(fd[0], PERF_EVENT_IOC_RESET, 0);
	ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	result=instructions_million();
	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

	ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	/****************/
	/* Initial Read */
	/****************/

	read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
	if (read_result!=sizeof(long long)*READ_SIZE) {
		printf("Unexpected read size\n");
	}

	if (!quiet) {
		printf("Initial read\n");
		for(i=0;i<count[0];i++) {
			printf("\t%i Counted %lld\n",i,count[1+i]);
		}
	}

	/***************/
	/* second read */
	/***************/

	read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
	if (read_result!=sizeof(long long)*READ_SIZE) {
		printf("Unexpected read size\n");
	}

	if (!quiet) {
		printf("Second read, to be sure\n");
		for(i=0;i<count[0];i++) {
			printf("\t%i Counted %lld\n",i,count[1+i]);
		}
	}

	/****************/
	/* reset leader */
	/****************/

	ioctl(fd[0], PERF_EVENT_IOC_RESET, 0);

	read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
	if (read_result!=sizeof(long long)*READ_SIZE) {
		printf("Unexpected read size\n");
	}

	if (!quiet) {
		printf("After reset of group leader\n");
		for(i=0;i<count[0];i++) {
			printf("\t%i Counted %lld\n",i,count[1+i]);
		}
	}

	if (count[1]!=0) {
		if (!quiet) printf("ERROR! Reset of event 0 did not work\n");
		failures++;
	}

	if (count[2]==0) {
		if (!quiet) printf("ERROR! Reset of leader cleared child\n");
		failures++;
	}

	/*******************/
	/* Count some more */
	/*******************/

	ioctl(fd[0], PERF_EVENT_IOC_RESET, 0);
	ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	result=instructions_million();
	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

	ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
	if (read_result!=sizeof(long long)*READ_SIZE) {
		if (!quiet) printf("Unexpected read size\n");
	}

	if (!quiet) {
		printf("Count some more\n");
		for(i=0;i<count[0];i++) {
			printf("\t%i Counted %lld\n",i,count[1+i]);
		}
	}

	read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
	if (read_result!=sizeof(long long)*READ_SIZE) {
		printf("Unexpected read size\n");
	}


	/***************/
	/* reset child */
	/***************/

	ioctl(fd[1], PERF_EVENT_IOC_RESET, 0);

	read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
	if (read_result!=sizeof(long long)*READ_SIZE) {
		printf("Unexpected read size\n");
	}

	if (!quiet) {
		printf("After reset of group child\n");
		for(i=0;i<count[0];i++) {
			printf("\t%i Counted %lld\n",i,count[1+i]);
		}
	}

	if (count[1]==0) {
		if (!quiet) printf("ERROR! Reset of child cleared leader\n");
		failures++;
	}

	if (count[2]!=0) {
		if (!quiet) printf("ERROR! Reset of child did not work\n");
		failures++;
	}

	/*******************/
	/* Count some more */
	/*******************/

	ioctl(fd[0], PERF_EVENT_IOC_RESET, 0);
	ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	result=instructions_million();
	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

	ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
	if (read_result!=sizeof(long long)*READ_SIZE) {
		if (!quiet) printf("Unexpected read size\n");
	}

	if (!quiet) {
		printf("Count some more\n");
		for(i=0;i<count[0];i++) {
			printf("\t%i Counted %lld\n",i,count[1+i]);
		}
	}

	read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
	if (read_result!=sizeof(long long)*READ_SIZE) {
		printf("Unexpected read size\n");
	}

	/*************************************/
	/* reset leader, PERF_IOC_FLAG_GROUP */
	/*************************************/

	ioctl(fd[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);

	read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
	if (read_result!=sizeof(long long)*READ_SIZE) {
		printf("Unexpected read size\n");
	}

	if (!quiet) {
		printf("After reset of group leader with PERF_IOC_FLAG_GROUP\n");
		for(i=0;i<count[0];i++) {
			printf("\t%i Counted %lld\n",i,count[1+i]);
		}
	}

	if (count[1]!=0) {
		if (!quiet) {
			printf("ERROR! Reset of leader with "
				"PERF_IOC_FLAG_GROUP did not work\n");
		}
		failures++;
	}

	if (count[2]!=0) {
		if (!quiet) {
			printf("ERROR! Reset of leader with "
				"PERF_IOC_FLAG_GROUP did not clear child\n");
			}
		failures++;
	}

	/*********************/
	/* Count some more   */
	/*********************/

	ioctl(fd[0], PERF_EVENT_IOC_RESET, 0);
	ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	result=instructions_million();
	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

	ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
	if (read_result!=sizeof(long long)*READ_SIZE) {
		printf("Unexpected read size\n");
	}

	if (!quiet) {
		printf("Count some more\n");
		for(i=0;i<count[0];i++) {
			printf("\t%i Counted %lld\n",i,count[1+i]);
		}
	}

	read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
	if (read_result!=sizeof(long long)*READ_SIZE) {
		printf("Unexpected read size\n");
	}


	/****************************************/
	/* reset child with PERF_IOC_FLAG_GROUP */
	/****************************************/

	ioctl(fd[1], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);

	read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
	if (read_result!=sizeof(long long)*READ_SIZE) {
		printf("Unexpected read size\n");
	}

	if (!quiet) {
		printf("After reset of group child with PERF_IOC_FLAG_GROUP\n");
		for(i=0;i<count[0];i++) {
			printf("\t%i Counted %lld\n",i,count[1+i]);
		}
	}

	if (count[1]!=0) {
		if (!quiet) {
			printf("ERROR! Reset of child with PERF_IOC_FLAG_GROUP "
				"did not clear leader\n");
		}
		failures++;
	}

	if (count[2]!=0) {
		if (!quiet) {
			printf("ERROR! Reset of child with PERF_IOC_FLAG_GROUP "
				"did not work\n");
			}
		failures++;
	}

	for(i=0;i<EVENTS;i++) {
		close(fd[i]);
	}

	(void) ret;

	if (failures) {
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}
