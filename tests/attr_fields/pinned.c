/* pinned.c  */
/* Test the pinned bit */

/* by Vince Weaver   vincent.weaver@maine.edu */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include <sys/mman.h>

#include <sys/ioctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#define NUM_EVENTS	20

#define MILLIONS	500

int main(int argc, char** argv) {

	int ret,fd[NUM_EVENTS],quiet,i,j;
	int result;
	long long buffer[NUM_EVENTS];
	int errors=0;
	double error;
	int exact;

	struct perf_event_attr pe;

	char test_string[]="Testing pinned...";

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing the pinned bit.\n");
	}

	/*********************************/
	/* Test 1: 1 event, not-pinned   */
	/*********************************/

	if (!quiet) {
		printf("*** Testing 1 event not-pinned\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.exclude_kernel=1;
	pe.pinned=0;

	fd[0]=perf_event_open(&pe,0,-1,-1,0);
	if (fd[0]<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening event %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	for(j=0;j<MILLIONS;j++) {
		instructions_million();
	}

	/* disable counting */
	ret=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	result=read(fd[0],buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	close(fd[0]);

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!  Empty result\n");
		errors++;
	}

	error=display_error(buffer[0],buffer[0],buffer[0],
			MILLIONS*1000000ULL,1);

        if ((error > 1.0) || (error<-1.0)) {
		if (!quiet) printf("Error too high: %lf\n",error);
		test_fail(test_string);
	}


	/*********************************/
	/* Test 2: 1 event, pinned       */
	/*********************************/

	if (!quiet) {
		printf("*** Testing 1 event pinned\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.exclude_kernel=1;
	pe.pinned=1;

	fd[0]=perf_event_open(&pe,0,-1,-1,0);
	if (fd[0]<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening event %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	for(j=0;j<MILLIONS;j++) {
		instructions_million();
	}

	/* disable counting */
	ret=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	result=read(fd[0],buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	close(fd[0]);

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!  Empty result\n");
		errors++;
	}

	error=display_error(buffer[0],buffer[0],buffer[0],
			MILLIONS*1000000ULL,1);

        if ((error > 1.0) || (error<-1.0)) {
		if (!quiet) printf("Error too high: %lf\n",error);
		test_fail(test_string);
	}

	/*********************************/
	/* Test 3: X events, non-pinned  */
	/*********************************/

	if (!quiet) {
		printf("*** Testing %d events, non pinned\n",NUM_EVENTS);
	}

	exact=0;

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.exclude_kernel=1;
	pe.pinned=0;
	pe.disabled=1;
	pe.read_format=PERF_FORMAT_TOTAL_TIME_ENABLED |
			PERF_FORMAT_TOTAL_TIME_RUNNING;

	for(i=0;i<NUM_EVENTS;i++) {
		fd[i]=perf_event_open(&pe,0,-1,-1,0);
		if (fd[i]<0) {
			if (!quiet) {
				fprintf(stderr,"Error opening event %d\n",i);
			}
			test_fail(test_string);
		}
	}

	/* enable counting */
	for(i=0;i<NUM_EVENTS;i++) {
		ret=ioctl(fd[i], PERF_EVENT_IOC_ENABLE,0);
		if (ret<0) {
			if (!quiet) printf("Error enabling %d\n",i);
		}
	}

	for(j=0;j<MILLIONS;j++) {
		instructions_million();
	}

	/* disable counting */
	for(i=0;i<NUM_EVENTS;i++) {
		ret=ioctl(fd[i], PERF_EVENT_IOC_DISABLE,0);
		if (ret<0) {
			if (!quiet) printf("Error disabling %d\n",i);
		}
	}

	for(i=0;i<NUM_EVENTS;i++) {
		result=read(fd[i],buffer,3*sizeof(long long));
		if (!quiet) {
			if (result<3) printf("Not counted\n");
			else {
				printf("\t%d %lld %lld %lld\n",i,buffer[0],buffer[1],buffer[2]);
			}
		}
		if (buffer[1]==buffer[2]) exact++;
		close(fd[i]);
	}

	if (exact>0) {
		test_fail("Unexpected exact results!\n");
	}

	/*********************************/
	/* Test 4: X events, one pinned  */
	/*********************************/

	if (!quiet) {
		printf("*** Testing %d events, one pinned\n",NUM_EVENTS);
	}

	exact=0;

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.exclude_kernel=1;
	pe.pinned=0;
	pe.disabled=1;
	pe.read_format=PERF_FORMAT_TOTAL_TIME_ENABLED |
			PERF_FORMAT_TOTAL_TIME_RUNNING;

	for(i=0;i<NUM_EVENTS;i++) {
		if (i==0) pe.pinned=1;
		else pe.pinned=0;

		fd[i]=perf_event_open(&pe,0,-1,-1,0);
		if (fd[i]<0) {
			if (!quiet) {
				fprintf(stderr,"Error opening event %d\n",i);
			}
			test_fail(test_string);
		}
	}

	/* enable counting */
	for(i=0;i<NUM_EVENTS;i++) {
		ret=ioctl(fd[i], PERF_EVENT_IOC_ENABLE,0);
		if (ret<0) {
			if (!quiet) printf("Error enabling %d\n",i);
		}
	}

	for(j=0;j<MILLIONS;j++) {
		instructions_million();
	}

	/* disable counting */
	for(i=0;i<NUM_EVENTS;i++) {
		ret=ioctl(fd[i], PERF_EVENT_IOC_DISABLE,0);
		if (ret<0) {
			if (!quiet) printf("Error disabling %d\n",i);
		}
	}

	for(i=0;i<NUM_EVENTS;i++) {
		result=read(fd[i],buffer,3*sizeof(long long));
		if (!quiet) {
			if (result<3) printf("Not counted\n");
			else {
				printf("\t%d %lld %lld %lld\n",i,buffer[0],buffer[1],buffer[2]);
			}
		}
		if (buffer[1]==buffer[2]) {
			exact++;
			error=display_error(buffer[0],buffer[0],buffer[0],
				MILLIONS*1000000ULL,1);

		        if ((error > 1.0) || (error<-1.0)) {
				if (!quiet) printf("Error too high: %lf\n",error);
				test_fail(test_string);
			}
		}
		close(fd[i]);
	}

	if (exact!=1) {
		test_fail("Unexpected exact results!\n");
	}

	/*********************************/
	/* Test 5: X events, all pinned  */
	/*********************************/

	if (!quiet) {
		printf("*** Testing %d events, all pinned\n",NUM_EVENTS);
	}

	exact=0;

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.exclude_kernel=1;
	pe.pinned=1;
	pe.disabled=1;
	pe.read_format=PERF_FORMAT_TOTAL_TIME_ENABLED |
			PERF_FORMAT_TOTAL_TIME_RUNNING;

	for(i=0;i<NUM_EVENTS;i++) {

		fd[i]=perf_event_open(&pe,0,-1,-1,0);
		if (fd[i]<0) {
			if (!quiet) {
				fprintf(stderr,"Error opening event %d\n",i);
			}
			test_fail(test_string);
		}
	}

	/* enable counting */
	for(i=0;i<NUM_EVENTS;i++) {
		ret=ioctl(fd[i], PERF_EVENT_IOC_ENABLE,0);
		if (ret<0) {
			if (!quiet) printf("Error enabling %d\n",i);
		}
	}

	for(j=0;j<MILLIONS;j++) {
		instructions_million();
	}

	/* disable counting */
	for(i=0;i<NUM_EVENTS;i++) {
		ret=ioctl(fd[i], PERF_EVENT_IOC_DISABLE,0);
		if (ret<0) {
			if (!quiet) printf("Error disabling %d\n",i);
		}
	}

	for(i=0;i<NUM_EVENTS;i++) {
		result=read(fd[i],buffer,3*sizeof(long long));
		if (!quiet) {
			if (result<3) printf("\tNot counted\n");
			else {
				printf("\t%d %lld %lld %lld\n",i,buffer[0],buffer[1],buffer[2]);
			}
		}
		if ((result==3) && (buffer[1]!=buffer[2])) exact++;

		if ((result==3) && (buffer[1]==buffer[2])) {

			error=display_error(buffer[0],buffer[0],buffer[0],
				MILLIONS*1000000ULL,1);

		        if ((error > 1.0) || (error<-1.0)) {
				if (!quiet) printf("Error too high: %lf\n",error);
				test_fail(test_string);
			}
		}

		close(fd[i]);
	}

	if (exact) {
		test_fail("Unexpected inexact results!\n");
	}



	/* Done */

	test_pass(test_string);

	return 0;
}

