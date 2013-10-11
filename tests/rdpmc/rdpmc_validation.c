/* This file attempts to validate rdpmc counter access  */
/* This feature reas introduced with Linux 3.4          */

/* by Vince Weaver, vincent.weaver _at_ maine.edu       */


char test_string[]="Testing if userspace rdpmc reads give expected results...";
int quiet=0;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#include <sys/mman.h>


#define MAX_EVENTS 16

#if defined(__i386__) || defined (__x86_64__)

static unsigned long long rdtsc(void) {

	unsigned a,d;

	__asm__ volatile("rdtsc" : "=a" (a), "=d" (d));

	return ((unsigned long long)a) | (((unsigned long long)d) << 32);
}

static unsigned long long rdpmc(unsigned int counter) {

	unsigned int low, high;

	__asm__ volatile("rdpmc" : "=a" (low), "=d" (high) : "c" (counter));

	return (unsigned long long)low | ((unsigned long long)high) <<32;
}

#define barrier() __asm__ volatile("" ::: "memory")

#else

static unsigned long long rdtsc(void) {

	return 0;

}

static unsigned long long rdpmc(unsigned int counter) {

	return 0;
}

#define barrier()

#endif


/* From Peter Zijlstra's demo code */
static unsigned long long mmap_read_self(void *addr,
					unsigned long long *enabled,
					unsigned long long *running) {

	struct perf_event_mmap_page *pc = addr;
	unsigned int seq;
	unsigned long long count, delta=0;

  do {
  again:
    seq=pc->lock;
    barrier();
    if (seq&1) goto again;

    if ((enabled || running) && pc->time_mult) {
      unsigned long long cyc = rdtsc();
      unsigned long long rem, quot;

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
  if (!quiet) printf("FAIL FAIL FAIL\n");
  return -1;

}


int main(int argc, char **argv) {

	int i,result;
	long page_size=getpagesize();
	double error;

	long long start_before,stop_after;

	void *addr[MAX_EVENTS];

	struct perf_event_attr pe;
	struct perf_event_mmap_page *our_mmap;

	int fd[MAX_EVENTS],ret1,ret2;

	unsigned long long stamp[MAX_EVENTS],stamp2[MAX_EVENTS];
	unsigned long long now[MAX_EVENTS],now2[MAX_EVENTS];

	int count=2;

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if userspace rdpmc() style reads work.\n\n");
	}


#if defined(__i386__) || defined (__x86_64__)
#else
	if (!quiet) printf("Test is x86 specific for now...\n");
	test_skip(test_string);
#endif

	/*****************************/
	/* TEST START/WORK/READ/STOP */
	/*****************************/

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);

	fd[0]=-1;

	for(i=0;i<count;i++) {
		pe.config=PERF_COUNT_HW_INSTRUCTIONS;

		if (i==0) {
			pe.disabled=1;
			pe.pinned=1;
		}
		else {
			pe.disabled=0;
			pe.pinned=0;
		}

		fd[i]=perf_event_open(&pe,0,-1,fd[0],0);
		if (fd[i]<0) {
			fprintf(stderr,"Error opening event %d\n",i);
			test_fail(test_string);
		}

		addr[i]=mmap(NULL,page_size, PROT_READ, MAP_SHARED,fd[i],0);
		if (addr[i] == (void *)(-1)) {
			fprintf(stderr,"Error mmap()ing event %d!\n",i);
			test_fail(test_string);
		}
		our_mmap=(struct perf_event_mmap_page *)addr[i];
		if (our_mmap->cap_user_rdpmc==0) {
			if (!quiet) printf("rdpmc support not detected\n");
			test_skip(test_string);
		}
	}


	/* start */
	start_before=rdtsc();

	ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	result=instructions_million();
	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

	/* read */
	for(i=0;i<count;i++) {
		stamp[i] = mmap_read_self(addr[i], NULL, &stamp2[i]);
		//now[i] = mmap_read_self(addr[i],NULL,&now2[i]);
	}

	/* stop */
	ret2=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	stop_after=rdtsc();

	if (ret1<0) {
		fprintf(stderr,"Error starting!\n");
		test_fail(test_string);
	}

	if (ret2<0) {
		fprintf(stderr,"Error stopping!\n");
		test_fail(test_string);
	}

	if (stamp[0]<0) {
		if (!quiet) printf("rdpmc support not available.\n");
		test_yellow_no(test_string);
	}

	if (!quiet) {
		printf("total start/read/stop latency: %lld cycles\n",
			stop_after-start_before);
		for(i=0;i<count;i++) {
			printf("\tEvent %x -- count: %lld running: %lld\n",
				i,stamp[i],stamp2[i]);
		}
	}

	for(i=0;i<count;i++) {
		close(fd[i]);
		munmap(addr[i],page_size);
	}

	/**********************************/
	/* TEST START/READ/WORK/READ/STOP */
	/**********************************/

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);

	fd[0]=-1;

	for(i=0;i<count;i++) {
		pe.config=PERF_COUNT_HW_INSTRUCTIONS;

		if (i==0) {
			pe.disabled=1;
			pe.pinned=1;
		}
		else {
			pe.disabled=0;
			pe.pinned=0;
		}

		fd[i]=perf_event_open(&pe,0,-1,fd[0],0);
		if (fd[i]<0) {
			fprintf(stderr,"Error opening event %d\n",i);
					test_fail(test_string);

		}

		addr[i]=mmap(NULL,page_size, PROT_READ, MAP_SHARED,fd[i],0);
		if (addr[i] == (void *)(-1)) {
			fprintf(stderr,"Error mmap()ing event %d!\n",i);
			test_fail(test_string);
		}
	}

	/* start */
	start_before=rdtsc();

	ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	for(i=0;i<count;i++) {
		stamp[i] = mmap_read_self(addr[i], NULL, &stamp2[i]);
	}

	result=instructions_million();
	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

	/* read */
	for(i=0;i<count;i++) {
		now[i] = mmap_read_self(addr[i],NULL,&now2[i]);
	}

	/* stop */
	ret2=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	stop_after=rdtsc();

	if (ret1<0) {
		fprintf(stderr,"Error starting!\n");
		test_fail(test_string);
	}

	if (ret2<0) {
		fprintf(stderr,"Error stopping!\n");
		test_fail(test_string);
	}

	if (stamp[0]<0) {
		if (!quiet) printf("rdpmc support not available.\n");
		test_yellow_no(test_string);
	}

	if (!quiet) {
		printf("total start/read/work/read/stop latency: %lld cycles\n",
			stop_after-start_before);
		for(i=0;i<count;i++) {
			printf("\tEvent %x -- count: %lld running: %lld\n",
				i,now[i]-stamp[i],now2[i]-stamp2[i]);
		}
	}

	if (!quiet) printf("\n");
	error=display_error(now[0]-stamp[0],
				now[0]-stamp[0],
				now[0]-stamp[0],
				1000000ULL,quiet);

	if ((error>1.0) || ( error<-1.0)) {
		if (!quiet) printf("Error out of range!\n");
		test_fail(test_string);
	}

	for(i=0;i<count;i++) {
		close(fd[i]);
		munmap(addr[i],page_size);
	}

	test_pass(test_string);

	return 0;
}
