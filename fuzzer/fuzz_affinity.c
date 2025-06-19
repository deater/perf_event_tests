#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <signal.h>
#include <sched.h>

#include "perf_event.h"

#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_stats.h"

#define MAX_CPUS 1024

void cpu_affinity_event(void) {

	static int initialized=0;

	pid_t pid=0;	/* current thread */
	static cpu_set_t *cpu_mask;
	int max_cpus=MAX_CPUS;
	size_t set_size;
	int cpu_array[MAX_CPUS];

	int i,j,which_one,result,num_set=0,which_cpu;

	/* TODO: get size of mask with getaffinity and multiply by 2? */

	/* only do this once */
	if (!initialized) {
		cpu_mask=CPU_ALLOC(max_cpus);
		if (cpu_mask==NULL) return;
		initialized=1;
	}

	set_size=CPU_ALLOC_SIZE(max_cpus);

	/* create new mask */
	which_one=rand()%4;
	switch (which_one) {
		/* set single core, low */
		case 0:
			num_set=1;
//			printf("%ld %p\n",set_size,cpu_mask);
			CPU_ZERO_S(set_size,cpu_mask);
			which_cpu=rand()%4;
			CPU_SET_S(which_cpu,set_size,cpu_mask);
			cpu_array[0]=which_cpu;
			break;
		/* set single core, full */
		case 1:
			num_set=1;
			CPU_ZERO_S(set_size,cpu_mask);
//			printf("Setting %d %d\n",new_value,max_cpus);
			which_cpu=rand()%max_cpus;
			CPU_SET_S(which_cpu,set_size,cpu_mask);
			cpu_array[0]=which_cpu;
			break;
		/* set lots of cores, low */
		case 2:
			num_set=0;
			CPU_ZERO_S(set_size,cpu_mask);
			for(j=0;j<32;j++) {
				if (rand()%2) {
					CPU_SET_S(j,max_cpus,cpu_mask);
					cpu_array[num_set]=j;
					num_set++;
				}
			}
			break;
		/* set lots of cores, high */
		case 3:
			num_set=0;
			CPU_ZERO_S(set_size,cpu_mask);
			for(j=0;j<max_cpus;j++) {
				if (rand()%2) {
					CPU_SET_S(j,max_cpus,cpu_mask);
					cpu_array[num_set]=j;
					num_set++;
				}
			}
			break;
		default: break;	// do nothing
	}

	stats.affinity_attempts++;
	stats.total_syscalls++;

	if (ignore_but_dont_skip.affinity) return;

	result=sched_setaffinity(pid,max_cpus,cpu_mask);

	/* urgh doing this properly is hard */

	char buffer[BUFSIZ];

	if (result==0) {
	        stats.affinity_successful++;
		if (logging&TYPE_AFFINITY) {
			sprintf(buffer,"a %d %d ",
				which_one,num_set);
			strncpy(log_buffer,buffer,BUFSIZ);
			for(i=0;i<num_set;i++) {
				sprintf(buffer,"%d ",cpu_array[i]);
				strncat(log_buffer,buffer,BUFSIZ-1);
			}
			strncat(log_buffer,"\n",BUFSIZ-1);
			write(log_fd,log_buffer,strlen(log_buffer));
		}

	}

}
