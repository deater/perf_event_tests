/* JP Kobryn reported that you could panic the kernel */
/* If LBR active while vsyscall running */
/* So let's try fuzzing that */


/* some notes:
        only room for 4 vsyscalls
        always appears at same memory location
        for security reasons this was bad, so they added vsyscall emulation
                that traps into kernel but is now slower
        deprecated in Linux 3.1
        replaced with vDSO

*/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <signal.h>

#include "perf_event.h"

#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_stats.h"

#include "fuzz_vsyscall.h"


/* 1 if found, 0 if not */
int detect_vsyscall(void) {
	FILE *fff;
	int result=0;
	char string[BUFSIZ];

	fff=fopen("/proc/self/maps","r");
	if (fff!=NULL) {
		while(1) {
			if (fgets(string,BUFSIZ,fff)==NULL) {
				break;
			}

			/* not very robust */
			if (strstr(string,"[vsyscall]")!=NULL) {
				result=1;
				break;
			}

		}
	}
	return result;

}

/* Note: which other architectures support this? */

#ifdef __x86_64__
int vsyscall_gettimeofday(struct timeval *tv) {

	register int64_t rax __asm__("rax") = 0xffffffffff600000;
	register void *rdi __asm__("rdi") = tv;

	__asm__ volatile (
		"callq *%%rax \n\t"
		: "+r" (rax)
		: "r" (rdi)
	);

	return rax;
}
#else
int vsyscall_gettimeofday(struct timeval *tv) {
	return 0;
}
#endif

#ifdef __x86_64__
time_t vsyscall_time(time_t *t) {

	register int64_t rax __asm__("rax") = 0xffffffffff600400;
	register void *rdi __asm__("rdi") = t;

	__asm__ volatile (
		"callq *%%rax \n\t"
		: "+r" (rax)
		: "r" (rdi)
	);

	return rax;
}
#else
time_t vsyscall_time(time_t *t) {
	return 0;
}
#endif

#ifdef __x86_64__
int vsyscall_getcpu(int *cpu, int *node) {

	register int64_t rax __asm__("rax") = 0xffffffffff600800;
	register void *rdi __asm__("rdi") = cpu;
	register void *rsi __asm__("rsi") = node;

	__asm__ volatile (
		"callq *%%rax \n\t"
		: "+r" (rax)
		: "r" (rdi), "r" (rsi)
	);

	return rax;
}
#else
int vsyscall_getcpu(int *cpu, int *node) {
	return 0;
}
#endif


void vsyscall_random_event(void) {

	int ret,i;
	struct timeval ours={0,0};
	int cpu,node;
	time_t our_time;
	int which;

	stats.vsyscall_attempts++;
	stats.total_syscalls++;

	if (ignore_but_dont_skip.vsyscall) return;

	which=rand()%3;

	if (kernel_features&KERNEL_VSYSCALL) {


		/* Make it more likely to hit this */
		ret=0;
		for(i=0;i<1000;i++) {


		switch(which) {
			case 0:
				vsyscall_gettimeofday(&ours);
				// printf("gettimeofday: %ld %ld\n",ours.tv_sec,ours.tv_usec);
				break;
			case 1:
				our_time=vsyscall_time(&our_time);
				// printf("time: %ld\n",our_time);
				break;
			case 2:
				vsyscall_getcpu(&cpu,&node);
				// printf("getcpu: %d %d\n",cpu,node);
				break;
		}
	}


	}


	sprintf(log_buffer,"V %d\n",which);

	if (ret==0) stats.vsyscall_successful++;
}
