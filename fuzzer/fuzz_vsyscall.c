/* JP Kobryn reported that you could panic the kernel */
/* If LBR active while vsyscall running */
/* So let's try fuzzing that */

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
