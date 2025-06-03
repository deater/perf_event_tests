/* Test if vdso works */

/* This isn't necessarily perf_event related */
/* but I check it during fuzzing and didn't have a good place for testing */
/* out the code */


/* some notes:

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/time.h>

#include "perf_event.h"


int main(int argc, char **argv) {

	struct timeval ours={0,0};
	unsigned int cpu,node;
	time_t our_time;

	void *vdso;

	typedef long (*gtod_t)(struct timeval *tv, struct timezone *tz);
	gtod_t vdso_gtod;

	typedef int (*vgettime_t)(clockid_t, struct timespec *);
	vgettime_t vdso_gettime;

	typedef long (*time_func_t)(time_t *t);
	time_func_t vdso_time;

	typedef long (*getcpu_t)(unsigned *, unsigned *, void *);
	getcpu_t vdso_getcpu;

	vdso = dlopen("linux-vdso.so.1", RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD);
	if (!vdso) {
		vdso = dlopen("linux-gate.so.1", RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD);
	}
	if (!vdso) {
		printf("warning: failed to find vDSO\n");
		return -1;
	}

	vdso_gtod = (gtod_t)dlsym(vdso, "__vdso_gettimeofday");
	if (!vdso_gtod) {
		printf("warning: failed to find gettimeofday in vDSO\n");
	}

	vdso_gettime = (vgettime_t)dlsym(vdso, "__vdso_clock_gettime");
	if (!vdso_gettime) {
		printf("warning: failed to find clock_gettime in vDSO\n");
	}

	vdso_time = (time_func_t)dlsym(vdso, "__vdso_time");
	if (!vdso_time) {
		printf("warning: failed to find time in vDSO\n");
	}

	vdso_getcpu = (getcpu_t)dlsym(vdso, "__vdso_getcpu");
	if (!vdso_getcpu) {
		printf("warning: failed to find getcpu in vDSO\n");
	}


	printf("Vdso found\n");

	vdso_gtod(&ours,NULL);
	printf("gettimeofday: %ld %ld\n",ours.tv_sec,ours.tv_usec);

	our_time=vdso_time(&our_time);
	printf("time: %ld\n",our_time);

	vdso_getcpu(&cpu,&node,NULL);
	printf("getcpu: %d %d\n",cpu,node);

	return 0;
}
