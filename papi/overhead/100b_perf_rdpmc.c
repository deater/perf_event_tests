/* This program measures the relative overhead of PAPI	*/
/* vs raw perf_event					*/

/* by Vince Weaver, vincent.weaver _at_ maine.edu	*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/mman.h>

#include "perf_event.h"
#include "perf_helpers.h"


#include "instructions_testcode.h"


#if defined(__i386__) || defined (__x86_64__)
#define barrier() __asm__ volatile("" ::: "memory")
#else
#define barrier()
#endif


inline unsigned long long rdtsc(void) {

        unsigned a,d;

        __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));

        return ((unsigned long long)a) | (((unsigned long long)d) << 32);
}

inline unsigned long long rdpmc(unsigned int counter) {

        unsigned int low, high;

        __asm__ volatile("rdpmc" : "=a" (low), "=d" (high) : "c" (counter));

        return (unsigned long long)low | ((unsigned long long)high) <<32;
}




#define NUM_RUNS 100000

long long results[NUM_RUNS];

static unsigned long long mmap_read_self(void *addr,
					unsigned long long *enabled,
					unsigned long long *running) {

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





int main(int argc, char **argv) {

	int fd;

	struct perf_event_attr pe;

	int i;
	int result;
	long long prev=0;

	long long total=0,average,max=0,min=0x7ffffffffffffffULL;
	unsigned long long counts[1],stamp2;


	void *mmap_area;

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
     		fprintf(stderr,"Error opening leader %llx %s\n",pe.config,strerror(errno));
		exit(1);
	}

	mmap_area=mmap(NULL,getpagesize(), PROT_READ, MAP_SHARED|MAP_POPULATE,fd,0);
	if (mmap_area == MAP_FAILED) {
		fprintf(stderr,"Error mmap()ing event\n");
		exit(1);
	}


	ioctl(fd, PERF_EVENT_IOC_RESET, 0);

	prev= mmap_read_self(mmap_area, NULL, &stamp2);
	printf("prev=%lld\n",prev);

	ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

	prev= mmap_read_self(mmap_area, NULL, &stamp2);
	printf("prev=%lld\n",prev);

	for(i=0;i<NUM_RUNS;i++) {

		result=instructions_million();

		counts[0] = mmap_read_self(mmap_area, NULL, &stamp2);


		results[i]=counts[0]-prev;
		prev=counts[0];

 	}

	ioctl(fd, PERF_EVENT_IOC_DISABLE,0);

	munmap(mmap_area,getpagesize());
	close(fd);

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
