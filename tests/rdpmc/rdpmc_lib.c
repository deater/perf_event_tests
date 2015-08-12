/* This file contains rdpmc access helper functions            */

/* This feature was introduced in Linux 3.4                    */
/* There were bugs in the detection interface until Linux 3.12 */
/*   with a major API/ABI change in 3.12                       */

/* by Vince Weaver, vincent.weaver _at_ maine.edu              */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"

#include "rdpmc_inlines.h"
#include "rdpmc_lib.h"

#include <sys/mman.h>


#define MAX_EVENTS 16

#if defined(__i386__) || defined (__x86_64__)

#define barrier() __asm__ volatile("" ::: "memory")

#else

#define barrier()

#endif


/* From Peter Zijlstra's demo code */
unsigned long long mmap_read_self(void *addr,
					 unsigned long long *enabled,
					 unsigned long long *running)
{
	struct perf_event_mmap_page *pc = addr;
	unsigned int seq;
	unsigned long long count, delta=0, cyc;
	unsigned long long rem, quot;

	do {
again:
		seq=pc->lock;
		barrier();
		if (seq&1) goto again;

		if ((enabled || running) && pc->time_mult) {
			cyc = rdtsc();


			quot=(cyc >> pc->time_shift);
			rem = cyc & ((1 << pc->time_shift) -1);
			delta=pc->time_offset +
				quot*pc->time_mult +
				((rem * pc->time_mult) >> pc->time_shift);
		}

		if (enabled) *enabled=pc->time_enabled+delta;
		if (running) *running=pc->time_running+delta;

		if (pc->index) {
			count=rdpmc(pc->index-1);
			count+=pc->offset;
		}
		else goto fail;

		barrier();
	} while (pc->lock != seq);

	return count;

fail:
	/* should do slow read here */
	return -1;

}


int detect_rdpmc(int quiet) {

	struct perf_event_attr pe;
	int fd;
	char *addr;
	struct perf_event_mmap_page *our_mmap;
	int page_size=getpagesize();

#if defined(__i386__) || defined (__x86_64__)
#else
        if (!quiet) printf("Test is x86 specific for now...\n");
	return 0;
#endif

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		fprintf(stderr,"Error opening event\n");
		return 0;
	}

	addr=mmap(NULL,page_size, PROT_READ, MAP_SHARED,fd,0);
	if (addr == MAP_FAILED) {
		fprintf(stderr,"Error mmap()ing event\n");
		return 0;
	}

	our_mmap=(struct perf_event_mmap_page *)addr;

	if (our_mmap->cap_user_rdpmc) {
		/* new 3.12 and newerrdpmc support found */
		return 1;
	}
	else {
		if ((!our_mmap->cap_bit0_is_deprecated) &&
			(our_mmap->cap_bit0)) {
			/* 3.4 to 3.11 broken support */
			return 2;
		}
	}

	if (!quiet) {
		printf("rdpmc support not detected (mmap->cap_user_rdpmc==%d)\n",
				our_mmap->cap_user_rdpmc);

	}

	return 0;
}
