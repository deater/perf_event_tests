/* This file contains rdpmc access helper functions            */

/* This feature was introduced in Linux 3.4                    */
/* There were bugs in the detection interface until Linux 3.12 */
/*   with a major API/ABI change in 3.12                       */
/* All the bugs weren't really ironed out until 4.13	       */

/* by Vince Weaver, vincent.weaver _at_ maine.edu              */


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
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


#if 0
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
#endif

/* based on the code in include/uapi/linux/perf_event.h */
inline unsigned long long mmap_read_self(void *addr,
					 unsigned long long *en,
					 unsigned long long *ru) {

	struct perf_event_mmap_page *pc = addr;

	uint32_t seq, time_mult, time_shift, index, width;
	uint64_t count, enabled, running;
	uint64_t cyc, time_offset;
	int64_t pmc = 0,pmc_before=0;
	uint64_t quot, rem;
	uint64_t delta = 0;


	do {
		/* The kernel increments pc->lock any time */
		/* perf_event_update_userpage() is called */
		/* So by checking now, and the end, we */
		/* can see if an update happened while we */
		/* were trying to read things, and re-try */
		/* if something changed */
		/* The barrier ensures we get the most up to date */
		/* version of the pc->lock variable */

		seq=pc->lock;
		barrier();

		/* For multiplexing */
		/* time_enabled is time the event was enabled */
		enabled = pc->time_enabled;
		/* time_running is time the event was actually running */
		running = pc->time_running;

		/* if cap_user_time is set, we can use rdtsc */
		/* to calculate more exact enabled/running time */
		/* for more accurate multiplex calculations */
		if ( (pc->cap_user_time) && (enabled != running)) {
			cyc = rdtsc();
			time_offset = pc->time_offset;
			time_mult = pc->time_mult;
			time_shift = pc->time_shift;

			quot=(cyc>>time_shift);
			rem = cyc & (((uint64_t)1 << time_shift) - 1);
			delta = time_offset + (quot * time_mult) +
				((rem * time_mult) >> time_shift);
		}
		enabled+=delta;

		/* actually do the measurement */

		/* Index of register to read */
		/* 0 means stopped/not-active */
		/* Need to subtract 1 to get actual index to rdpmc() */
		index = pc->index;

		/* count is the value of the counter the last time */
		/* the kernel read it */
		width = pc->pmc_width;
		count = pc->offset;
		count<<=(64-width);
		count>>=(64-width);

		/* Ugh, libpfm4 perf_event.h has cap_usr_rdpmc */
		/* while actual perf_event.h has cap_user_rdpmc */

		/* Only read if rdpmc enabled and event index valid */
		/* Otherwise return the older (out of date?) count value */
		if (pc->cap_user_rdpmc && index) {
			/* width can be used to sign-extend result */


			/* Read counter value */
			pmc_before = rdpmc(index-1);
			pmc=pmc_before;

			/* sign extend result */
			pmc<<=(64-width);
			pmc>>=(64-width);

			/* add current count into the existing kernel count */
			count+=pmc;

			running+=delta;
		}

		barrier();

	} while (pc->lock != seq);

//	printf("BLAH: pmc_raw=%lx pmc=%lx offset=%llx width=%d\n",
//		pmc_before,pmc,pc->offset,width);

	if (en) *en=enabled;
	if (ru) *ru=running;

	return count;
}


int detect_rdpmc(int quiet) {

	struct perf_event_attr pe;
	int fd;
	char *addr;
	struct perf_event_mmap_page *our_mmap;
	int page_size=getpagesize();

//#if defined(__i386__) || defined (__x86_64__)
//#else
//	if (!quiet) printf("Test is x86 specific for now...\n");
//	return 0;
//#endif

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
		/* new 3.12 and newer rdpmc support found */
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
