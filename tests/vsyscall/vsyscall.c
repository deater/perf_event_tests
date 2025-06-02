/* Test if vsyscall works */

/* This isn't necessarily perf_event related */
/* but I check it during fuzzing and didn't have a good place for testing */
/* out the code */


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


/* 1 if found, 0 if not */
static int detect_vsyscall(void) {
	FILE *fff;
	int result=0;
	char string[BUFSIZ];

	fff=fopen("/proc/self/maps","r");
	if (fff!=NULL) {
		while(1) {
			if (fgets(string,BUFSIZ,fff)==NULL) {
				break;
			}

//			printf("%s",string);

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


//typedef long (*time_func_t)(time_t *t);
//const time_func_t vtime = (time_func_t)VSYS(0xffffffffff600400);
//time_func_t vdso_time;

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

//typedef long (*getcpu_t)(unsigned *, unsigned *, void *);
//const getcpu_t vgetcpu = (getcpu_t)VSYS(0xffffffffff600800);
//getcpu_t vdso_getcpu;

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

int main(int argc, char **argv) {

	struct timeval ours={0,0};
	int vsyscall_available=0,cpu,node;
	time_t our_time;

	vsyscall_available=detect_vsyscall();

	if (vsyscall_available) {
		printf("Vsyscall (or emulation) found\n");
		vsyscall_gettimeofday(&ours);
		printf("gettimeofday: %ld %ld\n",ours.tv_sec,ours.tv_usec);
		our_time=vsyscall_time(&our_time);
		printf("time: %ld\n",our_time);
		vsyscall_getcpu(&cpu,&node);
		printf("getcpu: %d %d\n",cpu,node);
	}
	else {
		printf("No vsyscall support detected\n");
	}
	return 0;
}
