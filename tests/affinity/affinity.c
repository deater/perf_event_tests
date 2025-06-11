/* Test the sched_setaffinity() calls */

/* this is a horrible interface */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sched.h>

#include "perf_event.h"



int main(int argc, char **argv) {

	pid_t pid;
	cpu_set_t mask;
	cpu_set_t *mask2;
	int i,count,count2;
	int result;
	unsigned int which;

	pid=0;	// means current pid

	sched_getaffinity(pid,sizeof(mask),&mask);

	printf("Cpus in set: %d\n",CPU_COUNT(&mask));
	count=CPU_COUNT(&mask);
	for(i=0;i<count;i++) {
		if (CPU_ISSET(i,&mask)) printf("%d ",i);
	}
	printf("\n");

	count2=4096;
	mask2=CPU_ALLOC(count2);
	if (mask2==NULL) {
		printf("Error allocating mask!\n");
	}

	/* Try setting to zero */
	CPU_ZERO_S(count2,mask2);
	result=sched_setaffinity(pid,count2,mask2);
	if (result<0) {
		printf("Error setting empty affinity!\n");
	}

	/* Try adding CPU0  */
	getcpu(&which,NULL);
	printf("Trying to set affinity to CPU 0, currently %d\n",which);
	CPU_SET_S(0,count2,mask2);
	result=sched_setaffinity(pid,count2,mask2);
	if (result<0) {
		printf("Error setting CPU0 affinity!\n");
	}
	getcpu(&which,NULL);
	printf("Now running on %d\n",which);

	CPU_FREE(mask2);

	/* Try adding CPU1  */
	getcpu(&which,NULL);
	printf("Trying to set affinity to CPU 1, currently %d\n",which);
	CPU_ZERO_S(count2,mask2);
	CPU_SET_S(1,count2,mask2);
	result=sched_setaffinity(pid,count2,mask2);
	if (result<0) {
		printf("Error setting CPU1 affinity!\n");
	}
	usleep(100);
	getcpu(&which,NULL);
	printf("Now running on %d\n",which);

	CPU_FREE(mask2);


	return 0;
}
