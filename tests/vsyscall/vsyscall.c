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
/* TODO: add VDSO support */

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

#if 0
void vsyscall_random_event(void) {

	int ret,i;
	struct timeval tv;

	stats.vsyscall_attempts++;
	stats.total_syscalls++;

	if (ignore_but_dont_skip.vsyscall) return;

	/* Make it more likely to hit this */
	ret=0;
	for(i=0;i<1000;i++) {
		memset(&tv, 0, sizeof(tv));
		ret+=vsyscall_gettimeofday(&tv);
	}

	sprintf(log_buffer,"V 1\n");

	/* FIXME: are there others we should be trying? */

	if (ret==0) stats.vsyscall_successful++;
}
#endif

int main(int argc, char **argv) {

	struct timeval ours={0,0};
	int vsyscall_available=0;

	vsyscall_available=detect_vsyscall();

	if (vsyscall_available) {
		printf("Vsyscall (or emulation) found\n");
		vsyscall_gettimeofday(&ours);
		printf("%ld %ld\n",ours.tv_sec,ours.tv_usec);
	}
	else {
		printf("No vsyscall support detected\n");
	}
	return 0;
}
