/* This program measures the relative overhead of PAPI	*/
/* vs raw perf_event					*/

/* by Vince Weaver, vincent.weaver _at_ maine.edu	*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "perf_helpers.h"


#include "instructions_testcode.h"

#define NUM_RUNS 100000

#define NUM_EVENTS 2

long long results[NUM_RUNS];

int main(int argc, char **argv) {

	int fd[NUM_EVENTS];

	struct perf_event_attr pe;

	int i;
	int result;
	long long counts[NUM_EVENTS],prev=0;

	long long total=0,average,max=0,min=0x7ffffffffffffffULL;

	/* Setup Leader */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	fd[0]=perf_event_open(&pe,0,-1,-1,0);
	if (fd[0]<0) {
     		fprintf(stderr,"Error opening leader %llx %s\n",pe.config,strerror(errno));
		exit(1);
	}

	/* Setup other event */
	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_CPU_CYCLES;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	fd[1]=perf_event_open(&pe,0,-1,fd[0],0);
	if (fd[1]<0) {
     		fprintf(stderr,"Error opening leader %llx %s\n",pe.config,strerror(errno));
		exit(1);
	}


	/* Start leader */

	ioctl(fd[0], PERF_EVENT_IOC_RESET, 0);
	ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	for(i=0;i<NUM_RUNS;i++) {

		result=instructions_million();

		result=read(fd[0],&counts,sizeof(long long));

		insn_results[i]=counts[0]-prev_insn;
		prev_insn=counts[0];

		cycle_results[i]=counts[1]-prev_cycles;
		prev_cycles=counts[1];

 	}

	for(i=0;i<NUM_EVENTS;i++) {
		ioctl(fd[i], PERF_EVENT_IOC_DISABLE,0);
	}

	for(i=0;i<NUM_EVENTS;i++) {
		close(fd[i]);
	}

	for(i=0;i<NUM_RUNS;i++) {
		total+=results[i];
		if (results[i]>max) max=results[i];
		if (results[i]<min) min=results[i];
	}

	average=total/NUM_RUNS;
	printf("Average=%lld max=%lld min=%lld\n",average,max,min);

	(void) result;

	return 0;
}
