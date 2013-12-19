/* disabled.c  */
/* Test the various combinations of the disabled bit */

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

#define EVENTS 2

int main(int argc, char** argv) {

	int ret,fd1,fd2,quiet;
	int result;
	long long buffer[EVENTS];
	int errors=0;

	struct perf_event_attr pe;

	char test_string[]="Testing disabled bit...";

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing the disabled bit.\n");
	}

	/******************************/
	/******************************/
	/* Without enable             */
	/******************************/
	/******************************/

	if (!quiet) {
		printf("Without enable\n");
	}

	/*********************************/
	/* Test 1: 1 event, disabled = 0 */
	/*********************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	instructions_million();

	/* disable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	result=read(fd1,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("With enabled\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	close(fd1);

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	/*********************************/
	/* Test 2: 1 event, disabled = 1 */
	/*********************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	instructions_million();

	/* disable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	result=read(fd1,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("With disabled\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	close(fd1);

	if (buffer[0]!=0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	/***************************************/
	/* Test 3: 2 events, disabled/disabled */
	/***************************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd2=perf_event_open(&pe,0,-1,fd1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening child %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	instructions_million();

	/* disable leader */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	instructions_million();

	result=read(fd1,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("With disabled = disabled/disabled\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	if (buffer[0]!=0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	result=read(fd2,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tValue    [%d] : %lld\n",1,buffer[0]);
	}

	close(fd1);
	close(fd2);

	if (buffer[0]!=0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	/**************************************/
	/* Test 4: 2 events, enabled/disabled */
	/**************************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd2=perf_event_open(&pe,0,-1,fd1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening child %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	instructions_million();

	/* disable leader */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	instructions_million();

	result=read(fd1,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("With disabled = enabled/disabled\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	result=read(fd2,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tValue    [%d] : %lld\n",1,buffer[0]);
	}

	close(fd1);
	close(fd2);

	if (buffer[0]!=0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	/**************************************/
	/* Test 5: 2 events, disabled/enabled */
	/**************************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd2=perf_event_open(&pe,0,-1,fd1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening child %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	instructions_million();

	/* disable leader */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	instructions_million();

	result=read(fd1,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("With disabled = disabled/enabled\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	if (buffer[0]!=0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	result=read(fd2,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tValue    [%d] : %lld\n",1,buffer[0]);
	}

	close(fd1);
	close(fd2);

	if (buffer[0]!=0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	/**************************************/
	/* Test 6: 2 events, enabled/enabled  */
	/**************************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd2=perf_event_open(&pe,0,-1,fd1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening child %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	instructions_million();

	/* disable leader */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	instructions_million();

	result=read(fd1,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("With disabled = enabled/enabled\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	result=read(fd2,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tValue    [%d] : %lld\n",1,buffer[0]);
	}

	close(fd1);
	close(fd2);

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	/******************************/
	/******************************/
	/* With    enable             */
	/******************************/
	/******************************/

	if (!quiet) {
		printf("* With enable\n");
	}

	/*********************************/
	/* Test 1: 1 event, disabled = 0 */
	/*********************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	instructions_million();

	/* disable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	result=read(fd1,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("With enabled\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	close(fd1);

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}


	/*********************************/
	/* Test 2: 1 event, disabled = 1 */
	/*********************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	/* enable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error enabling\n");
	}


	instructions_million();

	/* disable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	result=read(fd1,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("With disabled\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	close(fd1);

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	/***************************************/
	/* Test 3: 2 events, disabled/disabled */
	/***************************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd2=perf_event_open(&pe,0,-1,fd1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening child %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	/* enable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error enabling\n");
	}

	instructions_million();

	/* disable leader */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	instructions_million();

	result=read(fd1,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("With disabled = disabled/disabled\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	result=read(fd2,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tValue    [%d] : %lld\n",1,buffer[0]);
	}

	close(fd1);
	close(fd2);

	if (buffer[0]!=0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	/**************************************/
	/* Test 4: 2 events, enabled/disabled */
	/**************************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd2=perf_event_open(&pe,0,-1,fd1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening child %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	/* enable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error enabling\n");
	}


	instructions_million();

	/* disable leader */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	instructions_million();

	result=read(fd1,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("With disabled = enabled/disabled\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	result=read(fd2,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tValue    [%d] : %lld\n",1,buffer[0]);
	}

	close(fd1);
	close(fd2);

	if (buffer[0]!=0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	/**************************************/
	/* Test 5: 2 events, disabled/enabled */
	/**************************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd2=perf_event_open(&pe,0,-1,fd1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening child %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	/* enable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error enabling\n");
	}

	instructions_million();

	/* disable leader */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	instructions_million();

	result=read(fd1,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("With disabled = disabled/enabled\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	result=read(fd2,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tValue    [%d] : %lld\n",1,buffer[0]);
	}

	close(fd1);
	close(fd2);

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	/**************************************/
	/* Test 6: 2 events, enabled/enabled  */
	/**************************************/

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe,quiet);

	fd2=perf_event_open(&pe,0,-1,fd1,0);
	if (fd1<0) {
		if (!quiet) {
			fprintf(stderr,"Error opening child %llx\n",pe.config);
		}
		test_fail(test_string);
	}

	/* enable counting */
	ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error enabling\n");
	}

	instructions_million();

	/* disable leader */
	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) printf("Error disabling\n");
	}

	instructions_million();

	result=read(fd1,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("With disabled = enabled/enabled\n");
		printf("\tValue    [%d] : %lld\n",0,buffer[0]);
	}

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	result=read(fd2,buffer,sizeof(long long));
	if (result<0) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (result!=sizeof(long long)) {
		fprintf(stderr,"Unexpected read result %d\n",result);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("\tValue    [%d] : %lld\n",1,buffer[0]);
	}

	close(fd1);
	close(fd2);

	if (buffer[0]==0) {
		if (!quiet) printf("\tError!\n");
		errors++;
	}

	/* done */

	if (errors) test_fail(test_string);

	test_pass(test_string);

	return 0;
}

